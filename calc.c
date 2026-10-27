// Nebin Sha Shabeek , 251ADB013
// https://github.com/Nebinsha/Arithmetic-Parser-in-C-nebin
// Compiled with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// resources used to do this: Gpt , gemini ( the assistance is in the code sections)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>

/*
 * AI assistnt: Chat gpt and gemini helped me to explain recursive descent parsing,global
 * character positions,and file/command-line handling.
 * Prompt:"explain a beginner C arithmetic parser using one
 * function for each operator precedence level,with exact error positions(mainly to understand then teach me to do it)."
 * I used these ideas to build and test this implementation.i also used
 * gemini for assistance to check these( used both to understand and for checking)
 */


typedef struct{
    double value;
    int is_float;
}Number;
typedef struct{
    const char *text;
    size_t length;
    size_t pos;
    size_t line_end;
    size_t error_pos;
    int has_error;
}Parser;
static const char student_name[]="Nebin";
static const char student_lastname[]="Shabeek";
static const char student_id[]="251ADB013";

static void set_error(Parser *p,size_t position)
{ if (!p->has_error) {
      p->has_error = 1;
        p->error_pos = position;}
}
static void skip_spaces(Parser *p)
{
    while (p->pos < p->line_end &&
  (p->text[p->pos]==' ' ||
    p->text[p->pos]=='\t' ||
    p->text[p->pos]=='\r')) {
        p->pos++;
    }
}
static Number parse_expression(Parser *p);

static Number make_number(double value, int is_float)
{ Number n;
    n.value=value;
    n.is_float=is_float;
    return n;
}
static Number parse_primary(Parser *p)
{
Number result=make_number(0.0, 0);
    skip_spaces(p);
    if (p->pos >=p->line_end) {
        set_error(p, p->line_end + 1);
        return result;
    }

    if(p->text[p->pos]=='(') {
    p->pos++;
     result=parse_expression(p);
        skip_spaces(p);

        if(p->has_error) {
            return result;
        }
        if (p->pos >= p->line_end || p->text[p->pos] != ')') {
            set_error(p, p->line_end + 1);
            return result;}
        p->pos++;
        return result;
    }
    if(isdigit((unsigned char)p->text[p->pos])) {
        size_t start=p->pos;
        int is_float=0;
        double value=0.0;

        while(p->pos < p->line_end &&
               isdigit((unsigned char)p->text[p->pos])) {
            value=value * 10.0 + (double)(p->text[p->pos] - '0');
            p->pos++;}

    if(p->pos < p->line_end && p->text[p->pos]=='.') {
        is_float = 1;
        p->pos++;

         if(p->pos>=p->line_end ||
            !isdigit((unsigned char)p->text[p->pos])){
            set_error(p, p->pos);
                return result; }
            {
                double place=0.1;
                while (p->pos < p->line_end &&
                       isdigit((unsigned char)p->text[p->pos])) {
                    value+=(double)(p->text[p->pos]-'0') * place;
                    place*=0.1;
                    p->pos++;
                }
            }
        }
        if(p->pos==start){
            set_error(p,p->pos + 1);
            return result;}

        return make_number(value, is_float);
    }
    set_error(p, p->pos + 1);
    return result;
}
/*power calls itself for the exponent,so the** is right-associative */
static Number parse_power(Parser *p)
{ 
     Number base = parse_primary(p);
        if(p->has_error){
            return base;
    }
  skip_spaces(p);
    if(p->pos + 1 < p->line_end &&
        p->text[p->pos]=='*' &&
        p->text[p->pos + 1]=='*') {
        size_t operator_pos=p->pos;
        size_t exponent_start;
        Number exponent;
        double answer;

        p->pos += 2;
        skip_spaces(p);
        exponent_start = p->pos + 1;
        exponent=parse_power(p);

    if(p->has_error){
         return base;
        }
    if(!base.is_float && !exponent.is_float &&
            exponent.value<0.0){
            set_error(p, exponent_start);
            return base;
        }
        answer=pow(base.value, exponent.value);

        if(!isfinite(answer)){
            set_error(p,operator_pos+1);
            return base;
        }

        return make_number(answer, base.is_float || exponent.is_float);}
   return base;
}
static Number parse_term(Parser *p)
{
    Number left=parse_power(p);
    while (!p->has_error) {
        char operation;
        size_t right_start;
        Number right;

        skip_spaces(p);
        if(p->pos >= p->line_end ||
          (p->text[p->pos] != '*' && p->text[p->pos] != '/')) {
            break;}
        /* a pair of stars belongs to exponentiation, not multiplication */
        if(p->text[p->pos]=='*' &&
            p->pos + 1 < p->line_end &&
            p->text[p->pos + 1] =='*') {
            break;
        }
    {
            size_t operator_pos= p->pos;
            operation =p->text[p->pos++];
            skip_spaces(p);
            right_start= p->pos + 1;
            right=parse_power(p);

        if(p->has_error){
                break;}
            if(operation == '/') {
                if(right.value == 0.0) {
                  set_error(p, right_start);
            break;
                }

                if(left.is_float || right.is_float) {
                    left=make_number(left.value / right.value, 1);
                }else{
 /* C integer division truncates toward zero. */
                    left=make_number((double)((long long)left.value /
                                                 (long long)right.value), 0);
                }
            }else{
                left=make_number(left.value * right.value,
                                   left.is_float || right.is_float);
            
                                }

            if(left.is_float && !isfinite(left.value)) {
                set_error(p, operator_pos + 1);
                break;
            }}
        continue;
    }
    return left;
}
static Number parse_expression(Parser *p)
{
    Number left=parse_term(p);
    while (!p->has_error){
        char operation;
        Number right;
        skip_spaces(p);

    if(p->pos>=p->line_end ||
            (p->text[p->pos]!='+' && p->text[p->pos] != '-')) {
        break;}
        operation=p->text[p->pos++];
        right=parse_term(p);


        if(p->has_error) {
            break;}

        if(operation=='+') {
            left=make_number(left.value + right.value,
                               left.is_float || right.is_float);
        }else{
            left=make_number(left.value - right.value,
                               left.is_float || right.is_float);
        }
    if(left.is_float && !isfinite(left.value)) {
            set_error(p, p->pos);
            break;}
    }
    return left;
}


static int append_text(char **buffer, size_t *used, size_t *capacity,
                       const char *text)
{
    size_t add=strlen(text);
    char *new_buffer;
    if(*used+add+1 > *capacity) {
  size_t new_capacity=(*capacity == 0) ? 256 : *capacity;

        while(new_capacity < *used+add + 1) {
            new_capacity *=2;
        }
        new_buffer=realloc(*buffer, new_capacity);
        if(new_buffer == NULL) {
      return 0;}

        *buffer=new_buffer;
        *capacity=new_capacity;
    }


    memcpy(*buffer + *used, text, add + 1);
    *used += add;
    return 1;
}


/* Read the whole file dynamically, so there is no fixed line-size limit. */
static char *read_file(const char *path, size_t *length)
{
    FILE *file=fopen(path, "rb");
    char *data=NULL;
    size_t used=0;
    size_t capacity=0;
    size_t count;
    char chunk[8192];

    if (file==NULL){
        return NULL;
    }

    while((count=fread(chunk, 1, sizeof(chunk),file)) > 0) {
        char *new_data;

   if(used + count + 1 > capacity){
            size_t new_capacity=(capacity==0) ? 8192 : capacity;
            while(new_capacity < used + count + 1){
                new_capacity *= 2;
     }

            new_data=realloc(data, new_capacity);
            if(new_data==NULL){
                free(data);
                fclose(file);
                return NULL;
            }

            data=new_data;
            capacity=new_capacity;
        }
        memcpy(data + used, chunk, count);
        used+=count; }

    if (ferror(file)){
        free(data);
        fclose(file);
        return NULL;
    }

    fclose(file);
    if(data ==NULL) {
        data=malloc(1);
        if(data==NULL) {
      return NULL;
        }
    }

    data[used]='\0';
    *length=used;
    return data;
}
static int is_directory(const char *path)
{struct stat info;
    return stat(path, &info)==0 && S_ISDIR(info.st_mode);
}
static int make_directory(const char *path)
{ if (mkdir(path,0777)==0 || is_directory(path)) {
        return 1;
    }
    return 0;
}
static const char *base_name(const char *path)
{ const char *slash=strrchr(path, '/');
    return slash==NULL ? path : slash + 1;
}
static void remove_txt_extension(char *name)
{
    size_t length=strlen(name);
    if(length >=4 && strcmp(name + length - 4, ".txt") == 0) {
        name[length - 4]= '\0';
    }
}
static int evaluate_file(const char *input_path, const char *output_dir)
{   size_t length=0;
    char *data =read_file(input_path, &length);
    char *results=NULL;
    size_t results_used=0;
    size_t results_capacity=0;
    size_t offset=0;
    char output_path[2048];
    char input_base[512];
    const char *base;
    FILE *output;


    if(data== NULL) {
        fprintf(stderr, "Error: cannot read input file '%s'\n", input_path);
        return 1;
    }
    base= base_name(input_path);
    if(strlen(base)>= sizeof(input_base)) {
        fprintf(stderr, "Error: input filename is too long\n");
        free(data);
        return 1;
    }
    strcpy(input_base, base);
    remove_txt_extension(input_base);

    if(!make_directory(output_dir)) {
        fprintf(stderr, "Error: cannot create output directory '%s'\n",
                output_dir);
  free(data);
        return 1;
    }
    if(snprintf(output_path,sizeof(output_path), "%s/%s_%s_%s_%s.txt",
                 output_dir, input_base, student_name, student_lastname,
                 student_id) >= (int)sizeof(output_path)) {
        fprintf(stderr, "Error: output path is too long\n");
        free(data);
        return 1;
    }

    while(offset < length){
        size_t line_end = offset;
        size_t first = offset;
        Parser parser;
        Number answer;
        char line_output[128];

        while(line_end < length && data[line_end] != '\n'){
            line_end++;
        }

        while(first < line_end &&
               (data[first]== ' ' || data[first]=='\t' ||
                data[first]== '\r')) {
            first++;
        }
        /* blank lines and lines whose first non-space character is # skip*/
        if(first != line_end && data[first]!='#') {
            parser.text=data;
            parser.length= length;
            parser.pos =offset;
            parser.line_end= line_end;
            parser.error_pos= 0;
            parser.has_error= 0;
            answer= parse_expression(&parser);
            skip_spaces(&parser);


            if(!parser.has_error && parser.pos < line_end) {
                set_error(&parser, parser.pos + 1);
            }
            if(parser.has_error) {
                char error_output[64];

          /*If any ofexpression fails, it discards all earlier answers */
                free(results);
                results=NULL;
                results_used= 0;
                results_capacity= 0;

              snprintf(error_output, sizeof(error_output),"ERROR:%zu\n",
                         parser.error_pos);
                if(!append_text(&results,&results_used, &results_capacity,
                                 error_output)) {
                    fprintf(stderr, "Error: not enough memory\n");
                    free(results);
                    free(data);
                    return 1;
                }
                break;
            }
            if (answer.is_float) {
                snprintf(line_output, sizeof(line_output), "%.15g\n",
                         answer.value);
            }else{
                snprintf(line_output, sizeof(line_output), "%.0f\n",
                         answer.value);
            }
            if(!append_text(&results,&results_used, &results_capacity,
                             line_output)) {
                fprintf(stderr, "Error: not enough memory\n");
                free(results);
                free(data);
                return 1;
            }
        }
        if (line_end==length) {
            offset=length;
        }else{
            offset=line_end + 1;
        }
    }
    if(results==NULL) {
        results=malloc(1);
        if (results == NULL) {
            free(data);
            return 1;
        }
        results[0] = '\0';
    }
    output= fopen(output_path, "w");
    if(output== NULL){
        fprintf(stderr, "Error: cannot create output file '%s'\n",
                output_path);
        free(results);
        free(data);
        return 1;
    }
    if(results_used > 0 &&
        fwrite(results, 1, results_used, output) != results_used) {
        fprintf(stderr, "Error: cannot write output file '%s'\n", output_path);
        fclose(output);
        free(results);
        free(data);
        return 1;
    }
    if(fclose(output) != 0) {
        fprintf(stderr, "Error: cannot finish output file '%s'\n", output_path);
        free(results);
        free(data);
        return 1;
    }
    free(results);
    free(data);
    return 0;
}
static int process_directory(const char *input_dir, const char *output_dir)
{ DIR *directory = opendir(input_dir);
    struct dirent *entry;
    int status= 0;
    if(directory== NULL) {
        fprintf(stderr, "error: cannot open input directory '%s'\n",
                input_dir);
        return 1;
    }
    while((entry = readdir(directory)) != NULL) {
        size_t name_length = strlen(entry->d_name);
        char input_path[2048];
        struct stat info;
        if(name_length < 4 ||
            strcmp(entry->d_name + name_length - 4, ".txt") != 0) {
            continue;
        }
        if (snprintf(input_path, sizeof(input_path), "%s/%s",
                     input_dir, entry->d_name) >= (int)sizeof(input_path)) {
            fprintf(stderr, "Error: input path is too long\n");
            status = 1;
            continue;
        }
        if(stat(input_path, &info) != 0 || !S_ISREG(info.st_mode)) {
            continue;
        }

     if(evaluate_file(input_path, output_dir) != 0) {
            status = 1;
        }
    }
    closedir(directory);
    return status;
}
int main(int argc, char *argv[])
{
    const char *input_path=NULL;
    const char *input_dir = NULL;
    const char *output_dir = NULL;
    int i;

    for (i = 1; i < argc; i++) {
        if((strcmp(argv[i], "-o") == 0 ||
             strcmp(argv[i], "--output-dir") == 0) && i + 1 < argc) {
            output_dir = argv[++i];
        }else if((strcmp(argv[i], "-d") == 0 ||
                    strcmp(argv[i], "--dir") == 0) && i + 1 < argc) {
            input_dir = argv[++i];
        }else if(argv[i][0] == '-') {
            fprintf(stderr, "Error: unknown option '%s'\n", argv[i]);
            return 1;
        }else if (input_path == NULL) {
            input_path = argv[i];
        }else{
            fprintf(stderr, "Error: unexpected argument '%s'\n", argv[i]);
            return 1;
        }
    }
 if((input_path==NULL && input_dir == NULL) ||
     (input_path != NULL && input_dir != NULL)) {
     fprintf(stderr,
                "usage: calc [-d DIR | --dir DIR] [-o OUTDIR | "
                "--output-dir OUTDIR] [input.txt]\n");
        return 1;
    }
if (input_dir!=NULL) {
        char default_dir[1024];
        if(output_dir == NULL){
            const char *base = base_name(input_dir);
            if(snprintf(default_dir, sizeof(default_dir),
                         "%s_%s_%s", base, student_name, student_id) >=
                (int)sizeof(default_dir)) {
                fprintf(stderr, "Error: default output path is too long\n");
                return 1;
            }
            output_dir = default_dir;
        }
        return process_directory(input_dir, output_dir);
    }
    if(output_dir== NULL) {
        char default_dir[1024];
        char input_base[512];
        const char *base = base_name(input_path);


        if (strlen(base) >= sizeof(input_base)) {
            fprintf(stderr, "Error: input filename is too long\n");
            return 1;
        }
        strcpy(input_base, base);
        remove_txt_extension(input_base);
        if (snprintf(default_dir, sizeof(default_dir), "%s_%s_%s",
                     input_base, student_name, student_id) >=
            (int)sizeof(default_dir)) {
            fprintf(stderr, "Error: default output path is too long\n");
            return 1;
        }


        output_dir= default_dir;
        return evaluate_file(input_path, output_dir);
    }
    return evaluate_file(input_path, output_dir);
}
