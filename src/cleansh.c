/*
// @(#) cleansh.c -- pass clean enviroment to process
*/

# include	<unistd.h>
# include	<stdlib.h>
# include	<string.h>
# include	<stdio.h>

enum	{
	ok	= 0,
	err	= -1,
};

enum	{
	false	= 0,
	true	= !false,
};

// Choose shell - rarely would be csh
# if 1
static	char	PATH_SHELL[]	= "/bin/sh";
static	char*	SHELL_ARGS[]	= { "--noprofile", 0};
# else
static	char	PATH_SHELL[]	= "/bin/csh";
static	char*	SHELL_ARGS[]	= { "-f", 0};
# endif

// csh uses (local)shell variable "prompt" ie not environment variable
// so this is ignored by csh.
static	char	SHELL_PROMPT[]	= "PS1";
static	char	PROMPT_STRING[]	= "cleansh ! $ ";

// Keep these envars
char*	env_vars[]	= {
	"USER",
	"LOGNAME",
	"HOME",
	"TERM",
	"LANG",
	"DISPLAY",
	0
};

//-----------------------------------------------------------------
// Simple vector of strings (char*) 
//
enum	{
	DEFAULT_STRVEC_SIZE	= 1024,
};
	
struct	string_vec	{
	size_t	size;
	size_t	used;
	char**	a;
};
typedef	struct	string_vec	STRVEC;

static	inline	char**	strvec_carray (STRVEC* sv) {
	return sv->a;
}
static	inline size_t	strvec_size (STRVEC* sv) {
	return	sv->size;
}
static	inline size_t	strvec_used (STRVEC* sv) {
	return	sv->used;
}
int	strvec_grow (STRVEC* sv) {
	int	result	= err;
	if (sv) {
		size_t	newsize	= strvec_size(sv) * 2;
		char**	new	= 0;
		if (newsize>0) {
			new	= realloc (strvec_carray(sv), newsize*sizeof(*new));
		}
		else {
			newsize	= DEFAULT_STRVEC_SIZE;
			new	= calloc (newsize,sizeof(*new));
		}
			
		if (new) {
			sv->a	= new;
			sv->size	= newsize;
			result	= ok;
		}
	}
	return	result;
}
int	strvec_Create (STRVEC** svp) {
	int	result	= err;
	STRVEC*	sv	= calloc (sizeof(*sv),1);
	if (sv) {
		sv->size	= 0;
		sv->used	= 0;
		sv->a		= 0;
		*svp	= sv;
		result	= ok;
	}
	return	result;
}
int	strvec_locate (STRVEC* sv, int (*cmp)(char*, char*), char* key) {
	int	result	= err;
	int	i	= 0;
	int	j	= strvec_used (sv);
	char**	arr	= strvec_carray (sv);
	while (i!=j) {
		if (cmp (key, arr[i])==0) {
			j	= i;
			result	= i;
		}
		else	{
			++i;
		}
	}
	return	result;
} 
//
//  replace s at position i in sv iff i < used(sv)
//
int	strvec_replace (STRVEC* sv, size_t idx, char* str) {
	int	result	= err;
	if (idx < strvec_used (sv)) {
		char**	arr	= strvec_carray (sv);
		char*	s	= arr[idx];
		if (strlen(s) < strlen (str)) {
			char*	t	= realloc (s, strlen(str)+1);
			if (t) {
				strcpy (t, str);
				arr [idx]	= t;
				result	= ok;
			}
		}
		else	{
			strcpy (s, str);
			result	= ok;
		}
	}
	return	result;
}
int	strvec_append (STRVEC* sv, char* str) {
	int	result	= err;
	if (strvec_used (sv) >= strvec_size (sv)) {
		result	= strvec_grow (sv);
		if (result == ok) {
			result	= strvec_append (sv, str);
		}
	}
	else if (str)	{
		char*	copy	= strdup (str);
		if (copy) {
			char**	arr	= strvec_carray (sv);
			arr [sv->used++]	= copy;
			result	= ok;
		}
	}
	else { // append null
		sv->a[sv->used++]	= str;
		result	= ok;
	}
	return	result;
}
//-------------------------------------------------------
static	int	keycmp (char* key, char* str) {
	size_t	keylen	= strlen(key);
	int	result	= strncmp (key, str, keylen);
	return	result == 0 ? '=' - str[keylen] : result;	
}
int	env_setenv (STRVEC* ee, char* key, char* value) {
	int	result	= err;
	int	located	= strvec_locate (ee, keycmp, key);

	size_t	keylen	= strlen(key);
	size_t	vallen	= strlen(value);
	char	line[keylen+1+vallen+1];

	strcpy (line, key);
	line[keylen]	= '=';
	strcpy(&line[keylen+1], value);

	if (located < 0) {
		result	= strvec_append (ee, line);
	}
	else	{
		result	= strvec_replace (ee, located, line);
	}
	return 	result;
}
int	env_copy (STRVEC* ee, char* env_vars[]) {
	int	result	= ok;
	size_t	i	= 0;
	char*	var	= 0;
	while ((var = env_vars[i])!=0) {
		char*	value	= getenv (var);
		if (value) {
			result	= env_setenv (ee, var, value);
			if (result != ok)
				return	result;
		}
		++i;
	}
	return	result;
}

static	char*	basename (char* p) {
	char*	result	= p;
	char*	t	= strrchr (p, '/');
	if (t) {
		result	= t+1;
	}
	return	result;
}
//-----------------------------------------------------------------
// Primative exception/error handling.
// Fail and exit if function doesn't return 'ok'
//
static	void inline	_FAIL (int retval, char* file, int line, char* msg) {
	if (retval != ok) {
		fprintf (stderr, "FATAL: %s(%d) %s\n", file, line, msg);
		exit (EXIT_FAILURE);
	}
}
//
// Need macro to expand FILE and LINE macros in place and stringize 
// offending function.
//
// To disable
// # define	FAIL(v)	(v)
//
# define	FAIL(v)	_FAIL((v),__FILE__, __LINE__, #v )

main (int argc, char* argv[], char** envp) {
	int	i	= 0;
	char*	shell		= PATH_SHELL;
	char**	shell_args	= SHELL_ARGS;
	char*	prompt		= SHELL_PROMPT;
	char*	prompt_string	= PROMPT_STRING;

	STRVEC*	newenv	= 0;
	STRVEC*	newargv	= 0;
	FAIL (
		strvec_Create (&newenv)
	);
	FAIL (
		strvec_Create (&newargv)
	);

	// Construct new environment
	FAIL (
		env_copy (newenv, env_vars)
	);
	FAIL (
		env_setenv (newenv, "SHELL", shell)
	);
	FAIL (
		env_setenv (newenv, "PATH", "/sbin:/bin:/usr/sbin:/usr/bin")
	);
	FAIL (
		env_setenv (newenv, "LC_COLLATE", "C")
	);
	FAIL (
		env_setenv (newenv, prompt, prompt_string)
	);
	
	FAIL (
		strvec_append (newenv, 0)
	);

	// Construct new argv
	FAIL (
		strvec_append (newargv, basename (shell))
	);
	for (i=0; shell_args[i] != 0; ++i) {
		FAIL (
			strvec_append (newargv, shell_args[i])
		);
	}

	for (i=1; i < argc ; ++i) { // append original args
		FAIL (
			strvec_append (newargv, argv[i])
		);
	}
	FAIL (
		strvec_append (newargv, 0)
	);
	execve (shell, strvec_carray(newargv), strvec_carray(newenv));
	exit (EXIT_FAILURE);
}
