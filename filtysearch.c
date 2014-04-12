#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <fftw3.h>
#include <math.h>

#define N 1024

#define MAX_CAND 100

typedef struct { int f; float p; } winpt_t;

static const winpt_t ath[] =
{
    { 0, 76.55 },
    { 20, 76.55 },
    { 25, 65.62 },
    { 31.5, 55.12 },
    { 40, 45.53 },
    { 50, 37.63 },
    { 63, 30.86 },
    { 80, 25.02 },
    { 100, 20.51 },
    { 125, 16.65 },
    { 160, 13.12 },
    { 200, 10.09 },
    { 250, 7.54 },
    { 315, 5.11 },
    { 400, 3.06 },
    { 500, 1.48 },
    { 630, 0.30 },
    { 800, -0.30 },
    { 1000, -0.01 },
    { 1250, 1.03 },
    { 1600, -1.19 },
    { 2000, -4.11 },
    { 2500, -7.05 },
    { 3150, -9.03 },
    { 4000, -8.49 },
    { 5000, -4.48 },
    { 6300, 3.28 },
    { 8000, 9.83 },
    { 10000, 10.48 },
    { 12500, 8.38 },
    { 16000, 25 },
    { 18000, 40 },
    { 192000, 40 },
};

float calc_score(double const *imp, double const *win, int len)
{
    int i;
    float accum = 0.0;

    for (i = 0; i < len; i++)
    {
        double e = pow(10.0, imp[i] - win[i]);
        accum += e * e;
    }

    return 1.0 / accum;
}

int main(int argc, char *argv[])
{
    winpt_t const *winptr = ath;
    char const *namefmt = "curve%03d.dat";
    float fs = 44100.0f;
    int results = 15;
    int delay = 4;
    int taps = 4;
    double win[N / 2];
    double db[N / 2];
    double cand[MAX_CAND][N];
    double *in = NULL;
    fftw_complex *out = NULL;
    fftw_plan plan = NULL;
    int opt;
    int i, j;

    while ((opt = getopt(argc, argv, "o:r:n:t:d:")) != -1)
        switch (opt)
        {
        case 'o': namefmt = optarg;                         break;
        case 'r': fs = atof(optarg);                        break;
        case 'n': results = atoi(optarg);                   break;
        case 't': taps = atoi(optarg);                      break;
        case 'd': delay = atoi(optarg);                     break;
        }

    srand(time(NULL));

    for (i = 0; i < N / 2; i++)
    {
        float f = i * fs / N;
        while (winptr[1].f <= f)
            winptr++;
        float r = winptr[0].p + (winptr[1].p - winptr[0].p)
                        * (f - winptr[0].f)
                        / (winptr[1].f - winptr[0].f);
        win[i] = r;
    }

    if ((in = fftw_malloc(sizeof(*in) * N)) == NULL
        || (out = fftw_malloc(sizeof(*out) * N)) == NULL
        || (plan = fftw_plan_dft_r2c_1d(N, in, out,
                FFTW_MEASURE | FFTW_PRESERVE_INPUT)) == NULL)
    {
        fprintf(stderr, "oh shit.\n");
        exit(EXIT_FAILURE);
    }

    float best = 0.0f;
    int count = 0;
    while (count < results)
    {
        memset(in, 0, sizeof(*in) * N);
        int meth = i < MAX_CAND ? 0 : rand() % 10;
        in[0] = -1.0f;
        switch (meth)
        {
        case 0:
            for (j = delay; j < delay + taps; j++)
            {
                in[j] = rand() * 3.0 / RAND_MAX - 1.5;
                in[j] = in[j] * in[j] * in[j];
            }
            break;
        case 1:
            for (j = delay; j < delay + taps; j++)
            {
                in[j] = cand[rand() % MAX_CAND][j];
                in[j] *= 0.75 + rand() * (0.5 / RAND_MAX);
            }
            break;
        case 2:
            for (j = delay; j < delay + taps; j++)
            {
                in[j] = cand[rand() % MAX_CAND][j];
                in[j] *= 0.95 + rand() * (0.1 / RAND_MAX);
            }
            break;
        case 3: case 4: case 5:  /* OMG! Three parents! */
            {
                int x = rand() % MAX_CAND;
                int y = rand() % (MAX_CAND - 1);
                int z = rand() % (MAX_CAND - 2);
                if (y >= x) y++;
                if (z >= x) z++;
                if (z >= y) z++;
                if (z == x) z++;
                for (j = delay; j < delay + taps; j++)
                {
                    in[j] = cand[rand() < (RAND_MAX / 3) ? x : rand() < (RAND_MAX / 2) ? y : z][j];
                    in[j] *= 0.95 + rand() * (0.1 / RAND_MAX);
                }
            }
            break;
        default:
            {
                int x = rand() % MAX_CAND;
                int y = rand() % (MAX_CAND - 1);
                if (y >= x) y++;
                for (j = delay; j < delay + taps; j++)
                {
                    in[j] = cand[rand() > (RAND_MAX / 2) ? x : y][j];
                    in[j] *= 0.95 + rand() * (0.1 / RAND_MAX);
                }
            }
            break;
        }

        fftw_execute(plan);

        /* Probably shouldn't waste time on logs before it's obvious the
         * result isn't junk.
         */
        for (j = 0; j < N / 2; j++)
            db[j] = log10(out[j][0] * out[j][0] + out[j][1] * out[j][1]) * 0.5 * 20.0;

        float score = calc_score(db, win, N / 2);
        if (best < score || i < MAX_CAND)
        {
            if (i >= MAX_CAND)
            {
                char name[256];
                sprintf(name, namefmt, ++count);
                printf("method %d: scored %f, writing to %s: ", meth, score, name);
                FILE *fptr = fopen(name, "wt");
                fprintf(fptr, "# ");
                for (j = 0; j < delay + taps; j++)
                {
                    fprintf(fptr, "%e,", in[j]);
                    printf("%f,", in[j]);
                }
                fprintf(fptr, "\n");
                printf("\n");
                for (j = 0; j < N / 2; j++)
                    fprintf(fptr, "%.0f %f %f\n", j * fs / N, db[j], win[j]);
                fclose(fptr);
            }
            best = score;
            memcpy(cand[i < MAX_CAND ? i : rand() % MAX_CAND], in, sizeof(cand[0]));
        }
    }
    return 0;
}

