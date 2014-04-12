CFLAGS=-O3 -pedantic -std=c99 -Wall -ffast-math -lm -lsndfile -lfftw3
TARGET=filtysearch

$(TARGET): $(TARGET).c
	$(CC) $< $(CFLAGS) -o $@

clean:
	rm -f $(TARGET) *.o

%.png: %.dat
	echo "set terminal png ; set output '$@'; plot '$<' using 1:2 with lines, '$<' using 1:3 with lines" | gnuplot

show-%: %.dat
	echo "set yrange [-15:30] ; plot '$<' using 1:2 with lines, '$<' using 1:3 with lines, '$<' using 1:(5*10**((\$$2-\$$3)/20)) with lines ; pause mouse keypress" | gnuplot

TRIES=64

delay1-%tap0$(TRIES).dat: $(TARGET)
	./$(TARGET) -d1 -t$* -n$(TRIES) -o 'delay1-$*tap%03d.dat'

delay2-%tap0$(TRIES).dat: $(TARGET)
	./$(TARGET) -d2 -t$* -n$(TRIES) -o 'delay2-$*tap%03d.dat'

delay4-%tap0$(TRIES).dat: $(TARGET)
	./$(TARGET) -d4 -t$* -n$(TRIES) -o 'delay4-$*tap%03d.dat'

delay6-%tap0$(TRIES).dat: $(TARGET)
	./$(TARGET) -d6 -t$* -n$(TRIES) -o 'delay6-$*tap%03d.dat'

delay8-%tap0$(TRIES).dat: $(TARGET)
	./$(TARGET) -d8 -t$* -n$(TRIES) -o 'delay8-$*tap%03d.dat'
