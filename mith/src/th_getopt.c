/*
(C) 2014 EEMBC(R).  All rights reserved.                            

All EEMBC Benchmark Software are products of EEMBC 
and are provided under the terms of the EEMBC Benchmark License Agreements.  
The EEMBC Benchmark Software are proprietary intellectual properties of EEMBC and its Members 
and is protected under all applicable laws, including all applicable copyright laws.  
If you received this EEMBC Benchmark Software without having 
a currently effective EEMBC Benchmark License Agreement, you must discontinue use. 
Please refer to LICENSE.md for the specific license agreement that pertains to this Benchmark Software.
*/

#include "th_lib.h"
#include "th_file.h"

char *th_optarg;
int th_optind = 1;

int __getopt_initialized;
static char *nextchar;
int opterr = 1;
int optopt = '?';
static int first_nonopt;
static int last_nonopt;
static enum
{
  REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
} ordering;

struct option
{
  const char *name;
  int has_arg;
  int *flag;
  int val;
};

#define SWAP_FLAGS(a,b)
# define my_index	th_strchr
#  define _(msgid)	(msgid)

static void
exchange (argv)
     char **argv;
{
  int bottom = first_nonopt;
  int middle = last_nonopt;
  int top = th_optind;
  char *tem;

  /* Exchange the shorter segment with the far end of the longer segment.
     That puts the shorter segment into the right place.
     It leaves the longer segment in the right place overall,
     but it consists of two parts that need to be swapped next.  */

  while (top > middle && middle > bottom)
    {
      if (top - middle > middle - bottom)
	{
	  /* Bottom segment is the short one.  */
	  int len = middle - bottom;
	  register int i;

	  /* Swap it with the top part of the top segment.  */
	  for (i = 0; i < len; i++)
	    {
	      tem = argv[bottom + i];
	      argv[bottom + i] = argv[top - (middle - bottom) + i];
	      argv[top - (middle - bottom) + i] = tem;
	      SWAP_FLAGS (bottom + i, top - (middle - bottom) + i);
	    }
	  /* Exclude the moved bottom segment from further swapping.  */
	  top -= len;
	}
      else
	{
	  /* Top segment is the short one.  */
	  int len = top - middle;
	  register int i;

	  /* Swap it with the bottom part of the bottom segment.  */
	  for (i = 0; i < len; i++)
	    {
	      tem = argv[bottom + i];
	      argv[bottom + i] = argv[middle + i];
	      argv[middle + i] = tem;
	      SWAP_FLAGS (bottom + i, middle + i);
	    }
	  /* Exclude the moved top segment from further swapping.  */
	  bottom += len;
	}
    }

  /* Update records for the slots the non-options now occupy.  */

  first_nonopt += (th_optind - last_nonopt);
  last_nonopt = th_optind;
}


static const char *
_getopt_initialize (argc, argv, optstring)
     int argc;
     char *const *argv;
     const char *optstring;
{
  /* Start processing options with ARGV-element 1 (since ARGV-element 0
     is the program name); the sequence of previously skipped
     non-option ARGV-elements is empty.  */

  first_nonopt = last_nonopt = th_optind;

  nextchar = NULL;

  /* Determine how to handle the ordering of options and nonoptions.  */

  if (optstring[0] == '-')
    {
      ordering = RETURN_IN_ORDER;
      ++optstring;
    }
  else if (optstring[0] == '+')
    {
      ordering = REQUIRE_ORDER;
      ++optstring;
    }
  else
    ordering = PERMUTE;

  return optstring;
}

int
th_getopt_long (argc, argv, optstring, longopts, longind)
     int argc;
     char *const *argv;
     const char *optstring;
     const struct option *longopts;
     int *longind;
{
  int long_only = 0;
  int print_errors = opterr;
  if (optstring[0] == ':')
    print_errors = 0;

  if (argc < 1)
    return -1;

  th_optarg = NULL;

  if (th_optind == 0 || !__getopt_initialized)
    {
      if (th_optind == 0)
	th_optind = 1;	/* Don't scan ARGV[0], the program name.  */
      optstring = _getopt_initialize (argc, argv, optstring);
      __getopt_initialized = 1;
    }

  /* Test whether ARGV[optind] points to a non-option argument.
     Either it does not have option syntax, or there is an environment flag
     from the shell indicating it is not an option.  The later information
     is only used when the used in the GNU libc.  */
# define NONOPTION_P (argv[th_optind][0] != '-' || argv[th_optind][1] == '\0')

  if (nextchar == NULL || *nextchar == '\0')
    {
      /* Advance to the next ARGV-element.  */

      /* Give FIRST_NONOPT & LAST_NONOPT rational values if OPTIND has been
	 moved back by the user (who may also have changed the arguments).  */
      if (last_nonopt > th_optind)
	last_nonopt = th_optind;
      if (first_nonopt > th_optind)
	first_nonopt = th_optind;

      if (ordering == PERMUTE)
	{
	  /* If we have just processed some options following some non-options,
	     exchange them so that the options come first.  */

	  if (first_nonopt != last_nonopt && last_nonopt != th_optind)
	    exchange ((char **) argv);
	  else if (last_nonopt != th_optind)
	    first_nonopt = th_optind;

	  /* Skip any additional non-options
	     and extend the range of non-options previously skipped.  */

	  while (th_optind < argc && NONOPTION_P)
	    th_optind++;
	  last_nonopt = th_optind;
	}

      /* The special ARGV-element `--' means premature end of options.
	 Skip it like a null option,
	 then exchange with previous non-options as if it were an option,
	 then skip everything else like a non-option.  */

      if (th_optind != argc && !th_strcmp (argv[th_optind], "--"))
	{
	  th_optind++;

	  if (first_nonopt != last_nonopt && last_nonopt != th_optind)
	    exchange ((char **) argv);
	  else if (first_nonopt == last_nonopt)
	    first_nonopt = th_optind;
	  last_nonopt = argc;

	  th_optind = argc;
	}

      /* If we have done all the ARGV-elements, stop the scan
	 and back over any non-options that we skipped and permuted.  */

      if (th_optind == argc)
	{
	  /* Set the next-arg-index to point at the non-options
	     that we previously skipped, so the caller will digest them.  */
	  if (first_nonopt != last_nonopt)
	    th_optind = first_nonopt;
	  return -1;
	}

      /* If we have come to a non-option and did not permute it,
	 either stop the scan or describe it to the caller and pass it by.  */

      if (NONOPTION_P)
	{
	  if (ordering == REQUIRE_ORDER)
	    return -1;
	  th_optarg = argv[th_optind++];
	  return 1;
	}

      /* We have found another option-ARGV-element.
	 Skip the initial punctuation.  */

      nextchar = (argv[th_optind] + 1
		  + (longopts != NULL && argv[th_optind][1] == '-'));
    }

  /* Decode the current option-ARGV-element.  */

  /* Check whether the ARGV-element is a long option.

     If long_only and the ARGV-element has the form "-f", where f is
     a valid short option, don't consider it an abbreviated form of
     a long option that starts with f.  Otherwise there would be no
     way to give the -f short option.

     On the other hand, if there's a long option "fubar" and
     the ARGV-element is "-fu", do consider that an abbreviation of
     the long option, just like "--fu", and not "-f" with arg "u".

     This distinction seems to be the most useful approach.  */

  if (longopts != NULL
      && (argv[th_optind][1] == '-'
	  || (long_only && (argv[th_optind][2] || !my_index (optstring, argv[th_optind][1])))))
    {
      char *nameend;
      const struct option *p;
      const struct option *pfound = NULL;
      int exact = 0;
      int ambig = 0;
      int indfound = -1;
      int option_index;

      for (nameend = nextchar; *nameend && *nameend != '='; nameend++)
	/* Do nothing.  */ ;

      /* Test all long options for either exact match
	 or abbreviated matches.  */
      for (p = longopts, option_index = 0; p->name; p++, option_index++)
	if (!th_strncmp (p->name, nextchar, nameend - nextchar))
	  {
	    if ((unsigned int) (nameend - nextchar)
		== (unsigned int) th_strlen (p->name))
	      {
		/* Exact match found.  */
		pfound = p;
		indfound = option_index;
		exact = 1;
		break;
	      }
	    else if (pfound == NULL)
	      {
		/* First nonexact match found.  */
		pfound = p;
		indfound = option_index;
	      }
	    else if (long_only
		     || pfound->has_arg != p->has_arg
		     || pfound->flag != p->flag
		     || pfound->val != p->val)
	      /* Second or later nonexact match found.  */
	      ambig = 1;
	  }

      if (ambig && !exact)
	{
	  if (print_errors)
	    th_fprintf (th_stderr, _("%s: option `%s' is ambiguous\n"),
		     argv[0], argv[th_optind]);
	  nextchar += th_strlen (nextchar);
	  th_optind++;
	  optopt = 0;
	  return '?';
	}

      if (pfound != NULL)
	{
	  option_index = indfound;
	  th_optind++;
	  if (*nameend)
	    {
	      /* Don't test has_arg with >, because some C compilers don't
		 allow it to be used on enums.  */
	      if (pfound->has_arg)
		th_optarg = nameend + 1;
	      else
		{
		  if (print_errors)
		    {
		      if (argv[th_optind - 1][1] == '-')
			/* --option */
			th_fprintf (th_stderr,
				 _("%s: option `--%s' doesn't allow an argument\n"),
				 argv[0], pfound->name);
		      else
			/* +option or -option */
			th_fprintf (th_stderr,
				 _("%s: option `%c%s' doesn't allow an argument\n"),
				 argv[0], argv[th_optind - 1][0], pfound->name);
		    }

		  nextchar += th_strlen (nextchar);

		  optopt = pfound->val;
		  return '?';
		}
	    }
	  else if (pfound->has_arg == 1)
	    {
	      if (th_optind < argc)
		th_optarg = argv[th_optind++];
	      else
		{
		  if (print_errors)
		    th_fprintf (th_stderr,
			   _("%s: option `%s' requires an argument\n"),
			   argv[0], argv[th_optind - 1]);
		  nextchar += th_strlen (nextchar);
		  optopt = pfound->val;
		  return optstring[0] == ':' ? ':' : '?';
		}
	    }
	  nextchar += th_strlen (nextchar);
	  if (longind != NULL)
	    *longind = option_index;
	  if (pfound->flag)
	    {
	      *(pfound->flag) = pfound->val;
	      return 0;
	    }
	  return pfound->val;
	}

      /* Can't find it as a long option.  If this is not getopt_long_only,
	 or the option starts with '--' or is not a valid short
	 option, then it's an error.
	 Otherwise interpret it as a short option.  */
      if (!long_only || argv[th_optind][1] == '-'
	  || my_index (optstring, *nextchar) == NULL)
	{
	  if (print_errors)
	    {
	      if (argv[th_optind][1] == '-')
		/* --option */
		th_fprintf (th_stderr, _("%s: unrecognized option `--%s'\n"),
			 argv[0], nextchar);
	      else
		/* +option or -option */
		th_fprintf (th_stderr, _("%s: unrecognized option `%c%s'\n"),
			 argv[0], argv[th_optind][0], nextchar);
	    }
	  nextchar = (char *) "";
	  th_optind++;
	  optopt = 0;
	  return '?';
	}
    }

  /* Look at and handle the next short option-character.  */

  {
    char c = *nextchar++;
    char *temp = my_index (optstring, c);

    /* Increment `th_optind' when we start to process its last character.  */
    if (*nextchar == '\0')
      ++th_optind;

    if (temp == NULL || c == ':')
      {
	if (print_errors)
	  {
	    /* 1003.2 specifies the format of this message.  */
	    th_fprintf (th_stderr, _("%s: illegal option -- %c\n"),
	             argv[0], c);
	  }
	optopt = c;
	return '?';
      }
    /* Convenience. Treat POSIX -W foo same as long option --foo */
    if (temp[0] == 'W' && temp[1] == ';')
      {
	char *nameend;
	const struct option *p;
	const struct option *pfound = NULL;
	int exact = 0;
	int ambig = 0;
	int indfound = 0;
	int option_index;

	/* This is an option that requires an argument.  */
	if (*nextchar != '\0')
	  {
	    th_optarg = nextchar;
	    /* If we end this ARGV-element by taking the rest as an arg,
	       we must advance to the next element now.  */
	    th_optind++;
	  }
	else if (th_optind == argc)
	  {
	    if (print_errors)
	      {
		/* 1003.2 specifies the format of this message.  */
		th_fprintf (th_stderr, _("%s: option requires an argument -- %c\n"),
			 argv[0], c);
	      }
	    optopt = c;
	    if (optstring[0] == ':')
	      c = ':';
	    else
	      c = '?';
	    return c;
	  }
	else
	  /* We already incremented `th_optind' once;
	     increment it again when taking next ARGV-elt as argument.  */
	  th_optarg = argv[th_optind++];

	/* th_optarg is now the argument, see if it's in the
	   table of longopts.  */

	for (nextchar = nameend = th_optarg; *nameend && *nameend != '='; nameend++)
	  /* Do nothing.  */ ;

	/* Test all long options for either exact match
	   or abbreviated matches.  */
	for (p = longopts, option_index = 0; p->name; p++, option_index++)
	  if (!th_strncmp (p->name, nextchar, nameend - nextchar))
	    {
	      if ((unsigned int) (nameend - nextchar) == th_strlen (p->name))
		{
		  /* Exact match found.  */
		  pfound = p;
		  indfound = option_index;
		  exact = 1;
		  break;
		}
	      else if (pfound == NULL)
		{
		  /* First nonexact match found.  */
		  pfound = p;
		  indfound = option_index;
		}
	      else
		/* Second or later nonexact match found.  */
		ambig = 1;
	    }
	if (ambig && !exact)
	  {
	    if (print_errors)
	      th_fprintf (th_stderr, _("%s: option `-W %s' is ambiguous\n"),
		       argv[0], argv[th_optind]);
	    nextchar += th_strlen (nextchar);
	    th_optind++;
	    return '?';
	  }
	if (pfound != NULL)
	  {
	    option_index = indfound;
	    if (*nameend)
	      {
		/* Don't test has_arg with >, because some C compilers don't
		   allow it to be used on enums.  */
		if (pfound->has_arg)
		  th_optarg = nameend + 1;
		else
		  {
		    if (print_errors)
		      th_fprintf (th_stderr, _("\
%s: option `-W %s' doesn't allow an argument\n"),
			       argv[0], pfound->name);

		    nextchar += th_strlen (nextchar);
		    return '?';
		  }
	      }
	    else if (pfound->has_arg == 1)
	      {
		if (th_optind < argc)
		  th_optarg = argv[th_optind++];
		else
		  {
		    if (print_errors)
		      th_fprintf (th_stderr,
			       _("%s: option `%s' requires an argument\n"),
			       argv[0], argv[th_optind - 1]);
		    nextchar += th_strlen (nextchar);
		    return optstring[0] == ':' ? ':' : '?';
		  }
	      }
	    nextchar += th_strlen (nextchar);
	    if (longind != NULL)
	      *longind = option_index;
	    if (pfound->flag)
	      {
		*(pfound->flag) = pfound->val;
		return 0;
	      }
	    return pfound->val;
	  }
	  nextchar = NULL;
	  return 'W';	/* Let the application handle it.   */
      }
    if (temp[1] == ':')
      {
	if (temp[2] == ':')
	  {
	    /* This is an option that accepts an argument optionally.  */
	    if (*nextchar != '\0')
	      {
		th_optarg = nextchar;
		th_optind++;
	      }
	    else
	      th_optarg = NULL;
	    nextchar = NULL;
	  }
	else
	  {
	    /* This is an option that requires an argument.  */
	    if (*nextchar != '\0')
	      {
		th_optarg = nextchar;
		/* If we end this ARGV-element by taking the rest as an arg,
		   we must advance to the next element now.  */
		th_optind++;
	      }
	    else if (th_optind == argc)
	      {
		if (print_errors)
		  {
		    /* 1003.2 specifies the format of this message.  */
		    th_fprintf (th_stderr,
			     _("%s: option requires an argument -- %c\n"),
			     argv[0], c);
		  }
		optopt = c;
		if (optstring[0] == ':')
		  c = ':';
		else
		  c = '?';
	      }
	    else
	      /* We already incremented `th_optind' once;
		 increment it again when taking next ARGV-elt as argument.  */
	      th_optarg = argv[th_optind++];
	    nextchar = NULL;
	  }
      }
    return c;
  }
}


