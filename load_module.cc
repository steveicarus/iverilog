/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "config.h"
# include  "util.h"
# include  "parse_api.h"
# include  "compiler.h"
# include  <iostream>
# include  <map>
# include  <cstdlib>
# include  <cstring>
# include  <string>
# include  <sys/types.h>
# include  <dirent.h>
# include  <cctype>
# include  <cassert>

/*
 * The module library items are maps of key names to file name within
 * the directory.
 */
struct module_library {
      char*dir;
      bool key_case_sensitive;
      map<string,const char*>name_map;
      struct module_library*next;
};

static struct module_library*library_list = 0;
static struct module_library*library_last = 0;

const char dir_character = '/';
extern FILE *depend_file;

/*
 * Use the type name as a key, and search the module library for a
 * file name that has that key.
 */
bool load_module(const char*type)
{
      char path[4096];
      char*ltype = strdup(type);

      for (char*tmp = ltype ; *tmp ;  tmp += 1)
	    *tmp = tolower(*tmp);

      for (struct module_library*lcur = library_list
		 ; lcur != 0 ;  lcur = lcur->next) {

	    const char*key = lcur->key_case_sensitive? type : ltype;
	    map<string,const char*>::const_iterator cur;
	    cur = lcur->name_map.find(key);
	    if (cur == lcur->name_map.end())
		  continue;

	    sprintf(path, "%s%c%s", lcur->dir, dir_character, (*cur).second);

	    if(depend_file) {
		  fprintf(depend_file, "%s\n", path);
		  fflush(depend_file);
	    }

	    if (ivlpp_string) {
		  char*cmdline = (char*)malloc(strlen(ivlpp_string) +
					       strlen(path) + 4);
		  strcpy(cmdline, ivlpp_string);
		  strcat(cmdline, " \"");
		  strcat(cmdline, path);
		  strcat(cmdline, "\"");
		  FILE*file = popen(cmdline, "r");

		  if (verbose_flag)
			cerr << "Executing: " << cmdline << endl;

		  pform_parse(path, file);
		  pclose(file);
		  free(cmdline);

	    } else {
		  if (verbose_flag)
			cerr << "Loading library file "
			     << path << "." << endl;

		  FILE*file = fopen(path, "r");
		  assert(file);
		  pform_parse(path, file);
		  fclose(file);
	    }

	    return true;
      }


      return false;
}

/*
 * This function takes the name of a library directory that the caller
 * passed, and builds a name index for it.
 */
int build_library_index(const char*path, bool key_case_sensitive)
{
      DIR*dir = opendir(path);
      if (dir == 0)
	    return -1;

      if (verbose_flag) {
	    cerr << "Indexing library: " << path << endl;
      }

      struct module_library*mlp = new struct module_library;
      mlp->dir = strdup(path);
      mlp->key_case_sensitive = key_case_sensitive;

	/* Scan the director for files. check each file name to see if
	   it has one of the configured suffixes. If it does, then use
	   the root of the name as the key and index the file name. */
      while (struct dirent*de = readdir(dir)) {
	    unsigned namsiz = strlen(de->d_name);
	    char*key = 0;

	    for (list<const char*>::iterator suf = library_suff.begin()
		       ; suf != library_suff.end()
		       ; suf ++ ) {
		  const char*sufptr = *suf;
		  unsigned sufsiz = strlen(sufptr);

		  if (sufsiz >= namsiz)
			continue;

		    /* If the directory is case insensitive, then so
		       is the suffix. */
		  if (key_case_sensitive) {
			if (strcmp(de->d_name + (namsiz-sufsiz),
				   sufptr) != 0)
			      continue;
		  } else {
			if (strcasecmp(de->d_name + (namsiz-sufsiz),
				       sufptr) != 0)
			      continue;
		  }

		  key = new char[namsiz-sufsiz+1];
		  strncpy(key, de->d_name, namsiz-sufsiz);
		  key[namsiz-sufsiz] = 0;

		  break;
	    }

	    if (key == 0)
		  continue;

	      /* If the key is not to be case sensitive, then change
		 it to lowercase. */
	    if (! key_case_sensitive)
		  for (char*tmp = key ;  *tmp ;  tmp += 1)
			*tmp = tolower(*tmp);

	    mlp->name_map[key] = strdup(de->d_name);
	    delete[]key;
      }

      closedir(dir);

      if (library_last) {
	    assert(library_list);
	    library_last->next = mlp;
	    mlp->next = 0;
	    library_last = mlp;
      } else {
	    library_list = mlp;
	    library_last = mlp;
	    mlp->next = 0;
      }

      return 0;
}

