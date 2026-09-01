objsdep = alloca.obj tradcpp.obj mkdeps.obj tradcif.obj hex.obj
objs = alloca.obj, tradcpp.obj, mkdeps.obj, tradcif.obj, hex.obj

all : tradcpp-debug.exe tradcpp.exe
	@ write sys$output ">>>"
	@ write sys$output ">>> La forza sia con te!"
	@ write sys$output ">>>"
	@ directory /size /date /width=filename=30 tradcpp*.exe
	@ write sys$output ""

tradcpp-debug.exe : $(objs)
	link /debug /exe=tradcpp-debug.exe $(objs)

tradcpp.exe : $(objs)
	link /exe=tradcpp.exe $(objs)

# MMS V4 on OpenVMS x86-64 does not reliably apply the historical
# user-defined .c.obj suffix rule when this description is copied into an
# out-of-tree build.  Keep the commands explicit so configure can bootstrap
# tradcpp from an entirely clean directory.
alloca.obj : alloca.c localshared.h
	cc /debug /nooptimize alloca.c

tradcpp.obj : tradcpp.c localshared.h
	cc /debug /nooptimize tradcpp.c

mkdeps.obj : mkdeps.c localshared.h
	cc /debug /nooptimize mkdeps.c

tradcif.obj : tradcif.c localshared.h
	cc /debug /nooptimize tradcif.c

hex.obj : hex.c localshared.h
	cc /debug /nooptimize hex.c

#- tradcif.c: tradcif.y
#- 	echo expect 40 shift/reduce conflicts
#- 	yacc $<
#- 	mv y.tab.c $@

clean :
	- delete tradcpp-debug.exe;*, tradcpp.exe;*, *.obj;*

#- realclean: clean
#- 	rm -f tradcif.c

# this assumes the directory is named TRADCPP
zip :
	purge
	set default [-]
	zip "-r" tradcpp.zip [.tradcpp]*.*
	set default [.tradcpp]
