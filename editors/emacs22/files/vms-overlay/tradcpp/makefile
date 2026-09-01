objs = tradcpp.o mkdeps.o tradcif.o hex.o



tradcpp: $(objs)
	cc -o $@ $(CFLAGS) $(objs)

.c.o:
	cc -c -o $@ $(CFLAGS) $<

tradcif.c: tradcif.y
	echo expect 40 shift/reduce conflicts
	yacc $<
	mv y.tab.c $@

clean:
	rm -f tradcpp *.o *~

realclean: clean
	rm -f tradcif.c

$(objs) : localshared.h

zip:
	dfull=`pwd` ; d=`basename $$dfull` ; \
	cd .. ; rm -f $$d.zip ; \
	zip -r $$d $$d

tgz:
	dfull=`pwd` ; d=`basename $$dfull` ; \
	cd .. ; rm -f $$d.tar.gz ; \
	tar cvzf $$d.tar.gz $$d

dist: zip tgz
