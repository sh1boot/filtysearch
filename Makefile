CFLAGS=-O3 -pedantic -std=c99 -Wall -ffast-math -lm -lsndfile -lfftw3
TARGET=filtysearch
WIDTH=512
HEIGHT=384

$(TARGET): $(TARGET).c
	$(CC) $< $(CFLAGS) -o $@

clean:
	rm -f $(TARGET) *.o *.dat

example: show-delay2-6tap016

%.svg: %.dat
	echo "set terminal svg size $(WIDTH),$(HEIGHT); set output '$@';" \
		"set xlabel 'frequency (Hz)' ;" \
		"set ylabel 'dB' ;" \
		"set yrange [-20:30] ;" \
		"set y2label 'badness' ;" \
		"set y2range [0:10] ;" \
		"set y2tics 0,1 ;" \
		"plot '$<' using 1:2 with lines title 'actual'," \
			" '$<' using 1:3 with lines title 'ideal'," \
			" '$<' using 1:(10**((\$$2-\$$3)/20)) axes x1y2 with lines title 'badness';" \
			"pause mouse keypress" | gnuplot

%.png: %.dat
	echo "set terminal png size $(WIDTH),$(HEIGHT) ; set output '$@';" \
		"set xlabel 'frequency (Hz)' ;" \
		"set ylabel 'dB' ;" \
		"set yrange [-20:30] ;" \
		"set y2label 'badness' ;" \
		"set y2range [0:10] ;" \
		"set y2tics 0,1 ;" \
		"plot '$<' using 1:2 with lines title 'actual'," \
			" '$<' using 1:3 with lines title 'ideal'," \
			" '$<' using 1:(10**((\$$2-\$$3)/20)) axes x1y2 with lines title 'badness';" \
			"pause mouse keypress" | gnuplot

show-%: %.dat
	echo "" \
		"set xlabel 'frequency (Hz)' ;" \
		"set ylabel 'dB' ;" \
		"set yrange [-20:30] ;" \
		"set y2label 'badness' ;" \
		"set y2range [0:10] ;" \
		"set y2tics 0,1 ;" \
		"plot '$<' using 1:2 with lines title 'actual'," \
			" '$<' using 1:3 with lines title 'ideal'," \
			" '$<' using 1:(10**((\$$2-\$$3)/20)) axes x1y2 with lines title 'badness';" \
			"pause mouse keypress" | gnuplot

define make-foo
delay$1-$2tap%.dat: $(TARGET)
	./$(TARGET) -d$1 -t$2 -n$$* -o 'delay$1-$2tap%03d.dat'
endef

.SECONDARY:

$(foreach d,1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16, \
	$(foreach t,1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16, \
		$(eval $(call make-foo,$d,$t))))
