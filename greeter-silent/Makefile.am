if DYNAMIC_GREETER
xdmlibdir = $(XDMLIBDIR)

xdmlib_LTLIBRARIES = libXdmGreetSilent.la

libXdmGreetSilent_la_SOURCES = \
		  greet-silent.c

libXdmGreetSilent_la_LIBADD = $(XDMGREET_LIBS)

AM_CFLAGS = $(CWARNFLAGS) $(XDMGREET_CFLAGS) -I$(top_srcdir)

libXdmGreetSilent_la_LDFLAGS = -module -avoid-version
endif

if LINT
ALL_LINT_FLAGS=$(LINT_FLAGS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
		$(AM_CPPFLAGS) $(CPPFLAGS)

lint:
	$(LINT) $(ALL_LINT_FLAGS) $(libXdmGreetSilent_la_SOURCES)

endif LINT
