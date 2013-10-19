#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <X11/Intrinsic.h>
#include <errno.h>

#include "dm.h"
#include "greet.h"

#define MAX_USERNAME 50
#define XDM_GREETER_CONFIG "XDM_GREETER_CONFIG"
#define DEFAULT_GREETER_CONFIG "/etc/X11/xdm/xdm-silent-greeter"

#ifdef GREET_LIB
/*
 * Function pointers filled in by the initial call ito the library
 */

int     (*__xdm_PingServer)(struct display *d, Display *alternateDpy) = NULL;
void    (*__xdm_SessionPingFailed)(struct display *d) = NULL;
void    (*__xdm_Debug)(const char * fmt, ...) = NULL;
void    (*__xdm_RegisterCloseOnFork)(int fd) = NULL;
void    (*__xdm_SecureDisplay)(struct display *d, Display *dpy) = NULL;
void    (*__xdm_UnsecureDisplay)(struct display *d, Display *dpy) = NULL;
void    (*__xdm_ClearCloseOnFork)(int fd) = NULL;
void    (*__xdm_SetupDisplay)(struct display *d) = NULL;
void    (*__xdm_LogError)(const char * fmt, ...) = NULL;
void    (*__xdm_SessionExit)(struct display *d, int status, int removeAuth) = NULL;
void    (*__xdm_DeleteXloginResources)(struct display *d, Display *dpy) = NULL;
int     (*__xdm_source)(char **environ, char *file) = NULL;
char    **(*__xdm_defaultEnv)(void) = NULL;
char    **(*__xdm_setEnv)(char **e, char *name, char *value) = NULL;
char    **(*__xdm_putEnv)(const char *string, char **env) = NULL;
char    **(*__xdm_parseArgs)(char **argv, char *string) = NULL;
void    (*__xdm_printEnv)(char **e) = NULL;
char    **(*__xdm_systemEnv)(struct display *d, char *user, char *home) = NULL;
void    (*__xdm_LogOutOfMem)(const char * fmt, ...) = NULL;
void    (*__xdm_setgrent)(void) = NULL;
struct group    *(*__xdm_getgrent)(void) = NULL;
void    (*__xdm_endgrent)(void) = NULL;
# ifdef USESHADOW
struct spwd   *(*__xdm_getspnam)(GETSPNAM_ARGS) = NULL;
#  ifndef QNX4
void   (*__xdm_endspent)(void) = NULL;
#  endif /* QNX4 doesn't use endspent */
# endif
struct passwd   *(*__xdm_getpwnam)(GETPWNAM_ARGS) = NULL;
# if defined(linux) || defined(__GLIBC__)
void   (*__xdm_endpwent)(void) = NULL;
# endif
char     *(*__xdm_crypt)(CRYPT_ARGS) = NULL;
# ifdef USE_PAM
pam_handle_t **(*__xdm_thepamhp)(void) = NULL;
# endif

#endif

static void init(struct dlfuncs *dlfuncs)
{
#ifdef GREET_LIB
/*
 * These must be set before they are used.
 */
    __xdm_PingServer = dlfuncs->_PingServer;
    __xdm_SessionPingFailed = dlfuncs->_SessionPingFailed;
    __xdm_Debug = dlfuncs->_Debug;
    __xdm_RegisterCloseOnFork = dlfuncs->_RegisterCloseOnFork;
    __xdm_SecureDisplay = dlfuncs->_SecureDisplay;
    __xdm_UnsecureDisplay = dlfuncs->_UnsecureDisplay;
    __xdm_ClearCloseOnFork = dlfuncs->_ClearCloseOnFork;
    __xdm_SetupDisplay = dlfuncs->_SetupDisplay;
    __xdm_LogError = dlfuncs->_LogError;
    __xdm_SessionExit = dlfuncs->_SessionExit;
    __xdm_DeleteXloginResources = dlfuncs->_DeleteXloginResources;
    __xdm_source = dlfuncs->_source;
    __xdm_defaultEnv = dlfuncs->_defaultEnv;
    __xdm_setEnv = dlfuncs->_setEnv;
    __xdm_putEnv = dlfuncs->_putEnv;
    __xdm_parseArgs = dlfuncs->_parseArgs;
    __xdm_printEnv = dlfuncs->_printEnv;
    __xdm_systemEnv = dlfuncs->_systemEnv;
    __xdm_LogOutOfMem = dlfuncs->_LogOutOfMem;
    __xdm_setgrent = dlfuncs->_setgrent;
    __xdm_getgrent = dlfuncs->_getgrent;
    __xdm_endgrent = dlfuncs->_endgrent;
# ifdef USESHADOW
    __xdm_getspnam = dlfuncs->_getspnam;
#  ifndef QNX4
    __xdm_endspent = dlfuncs->_endspent;
#  endif /* QNX4 doesn't use endspent */
# endif
    __xdm_getpwnam = dlfuncs->_getpwnam;
# if defined(linux) || defined(__GLIBC__)
    __xdm_endpwent = dlfuncs->_endpwent;
# endif
    __xdm_crypt = dlfuncs->_crypt;
# ifdef USE_PAM
    __xdm_thepamhp = dlfuncs->_thepamhp;
# endif
#endif
}

static char *envvars[] = {
    "TZ",			/* SYSV and SVR4, but never hurts */
#if defined(sony) && !defined(SYSTYPE_SYSV) && !defined(_SYSTYPE_SYSV)
    "bootdev",
    "boothowto",
    "cputype",
    "ioptype",
    "machine",
    "model",
    "CONSDEVTYPE",
    "SYS_LANGUAGE",
    "SYS_CODE",
#endif
#if (defined(SVR4) || defined(SYSV)) && defined(i386) && !defined(sun)
    "XLOCAL",
#endif
    NULL
};

static char **
userEnv (struct display *d, int useSystemPath, char *user, char *home, char *shell)
{
    char	**env;
    char	**envvar;
    char	*str;

    env = defaultEnv ();
    env = setEnv (env, "DISPLAY", d->name);
    env = setEnv (env, "HOME", home);
    env = setEnv (env, "LOGNAME", user); /* POSIX, System V */
    env = setEnv (env, "USER", user);    /* BSD */
    env = setEnv (env, "PATH", useSystemPath ? d->systemPath : d->userPath);
    env = setEnv (env, "SHELL", shell);

    for (envvar = envvars; *envvar; envvar++)
    {
	str = getenv(*envvar);
	if (str)
	    env = setEnv (env, *envvar, str);
    }
    return env;
}

static char username[MAX_USERNAME];

_X_EXPORT
greet_user_rtn GreetUser(
    struct display          *d,
    Display                 ** dpy,
    struct verify_info      *verify,
    struct greet_info       *greet,
    struct dlfuncs        *dlfuncs)
{
    init(dlfuncs);

    char *config_fn = getenv(XDM_GREETER_CONFIG);
    if (!config_fn)
        config_fn = DEFAULT_GREETER_CONFIG;

    FILE *config_fs = fopen(config_fn, "r");
    if (!config_fs) {
        LogError("Config file %s open failed: %s\n", config_fn, strerror(errno));
        SessionExit(d, UNMANAGE_DISPLAY, FALSE);
    }

    if (!fgets(username, MAX_USERNAME, config_fs)) {
        LogError("Reading config file %s failed\n", config_fn);
        SessionExit(d, UNMANAGE_DISPLAY, FALSE);
    }

    size_t username_len = strlen(username);
    if (username[username_len - 1] == '\n')
        username[--username_len] = 0;

    greet->name = username;
    struct passwd *p;
    Debug ("Verify %s ...\n", greet->name);
    p = getpwnam (greet->name);
    endpwent();

    if (!p || username_len == 0) {
	LogError("Wrong user: %s\n", username);
	SessionExit(d, OBEYSESS_DISPLAY, FALSE);
    }

    verify->uid = p->pw_uid;
    verify->gid = p->pw_gid;

    char **argv = NULL;
    if (d->session)
        argv = parseArgs (argv, d->session);
    if (greet->string)
        argv = parseArgs (argv, greet->string);
    if (!argv)
        argv = parseArgs (argv, "xsession");

    verify->argv = argv;
    verify->userEnviron = userEnv (d, p->pw_uid == 0,
				    greet->name, p->pw_dir, p->pw_shell);
    Debug ("user environment:\n");
    printEnv (verify->userEnviron);
    verify->systemEnviron = systemEnv (d, greet->name, p->pw_dir);
    Debug ("system environment:\n");
    printEnv (verify->systemEnviron);
    Debug ("end of environments\n");

    if (source (verify->systemEnviron, d->startup) != 0)
    {
	Debug ("Startup program %s exited with non-zero status\n",
		d->startup);
	SessionExit (d, OBEYSESS_DISPLAY, FALSE);
    }

    return Greet_Success;
}
