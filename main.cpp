#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <utime.h>
#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <iostream>
#include <vector>

/*
Optimal results found with these settings

#define MUTATION_RATE 0.02
#define CROSSOVR_RATE 0.7
#define POPULATN_SIZE 1000
#define GENERATN_SIZE 100000
*/

#define FILE_NAME "results.csv"

typedef unsigned long long ull;

static const char alphanum[] =
	"0123456789"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"- _,'?";

const unsigned int alphanum_len = strlen(alphanum);

std::string  string_target;
unsigned int string_length;

// optparse flags
int     GENERATN_SIZE = 100000;
int     POPULATN_SIZE = 1000;
float	MUTATION_RATE = 0.02f;
float	CROSSOVR_RATE = 0.7f;

bool elitism_flag  = false;
bool output_flag   = false;
bool verbose_flag  = false;
bool worked_flag   = false;

struct solution
{
	std::string text;
	unsigned int fitness;

	solution(std::string _t, unsigned int _f)
	{
		this->text = _t;
		this->fitness = _f;
	}
};

float rand_float()
{
	return (float)rand() / RAND_MAX;
}

unsigned int rand_int(unsigned int min, unsigned int max)
{
	return rand() % ( max + 1 - min) + min;
}

char rand_char()
{
	return alphanum[rand() % alphanum_len];
}

std::string generate_string()
{
	unsigned int i;
	std::string gen_string = "";

	for(i = 0; i < string_length; i++)
	{
		gen_string += rand_char();
	}

	return gen_string;
}

solution* create_solution(std::string _text, unsigned int _fitness)
{
	solution *_sol = new solution(_text, _fitness);
	return _sol;
}

solution* get_best_solution(std::vector<solution*>& _pop)
{
	unsigned int max = 0;
	unsigned int count = 0;
	unsigned int index = 0;

	for(solution* _sol : _pop)
	{
		if(max < _sol->fitness)
		{
			max = _sol->fitness;
			index = count;
		}

		count++;
	}

	return _pop[index];
}

std::string get_best_solution_text(std::vector<solution*>& _pop)
{
	unsigned int max = 0;
	unsigned int count = 0;
	unsigned int index = 0;

	for(solution* _sol : _pop)
	{
		if(max < _sol->fitness)
		{
			max = _sol->fitness;
			index = count;
		}

		count++;
	}

	return _pop[index]->text;
}

unsigned int calc_fitness(std::string& _str)
{
	unsigned int i;
	unsigned int count = 0;

	for(i = 0; i < string_target.length(); i++)
	{
		if(_str[i] == string_target[i])
		{
			count++;
		}
	}

	return count;
}

unsigned int get_max_fitness(std::vector<solution*>& _pop)
{
	unsigned int max = 0;

	for(solution *_sol : _pop)
	{
		if(_sol->fitness > max)
		{
			max = _sol->fitness;
		}
	}

	return max;
}

void print_sol_max_fitness(std::vector<solution*>& _pop)
{
	unsigned int max = 0;
	std::string max_str;

	for(solution *_sol : _pop)
	{
		if(_sol->fitness > max)
		{
			max = _sol->fitness;
			max_str = _sol->text;
		}
	}

	if(verbose_flag)
		std::cout << "[INFO] - Text: "
				  << max_str
				  << std::endl;
}

void population_print(std::vector<solution*>& _pop)
{
	if(verbose_flag)
		for(solution *_sol : _pop)
		{
			std::cout << "Text:"
					  << _sol->text
					  << " "
					  << "Fitness:"
					  << _sol->fitness
					  << std::endl;
		}
}

void population_init(std::vector<solution*>& _pop)
{
	int i;

	for(i = 0; i < POPULATN_SIZE; i++)
	{
		std::string _rands = generate_string();
		_pop.push_back(create_solution(_rands, calc_fitness(_rands)));
	}
}

void parents_init(std::vector<solution*>& _parents)
{
	int i;

	for(i = 0; i < POPULATN_SIZE; i++)
	{
		std::string _rands = generate_string();
		_parents.push_back(create_solution(_rands, calc_fitness(_rands)));
	}
}

void tournament(std::vector<solution*>& _pop, std::vector<solution*>& _parents)
{
	unsigned int rand_a;
	unsigned int rand_b;

	for(int i = 0; i < POPULATN_SIZE; i++)
	{
		rand_a = rand_int(0, POPULATN_SIZE - 1);

		do
		{
			rand_b = rand_int(0, POPULATN_SIZE - 1);
		} while(rand_a == rand_b);

		if(_pop[rand_a]->fitness > _pop[rand_b]->fitness)
		{
			_parents[i]->text = _pop[rand_a]->text;
			_parents[i]->fitness = _pop[rand_a]->fitness;
		}
		else
		{
			_parents[i]->text = _pop[rand_b]->text;
			_parents[i]->fitness = _pop[rand_b]->fitness;
		}
	}
}

void crossover(std::vector<solution*>& _pop, std::vector<solution*>& _parents)
{
	unsigned int point;

	for(int i = 0; i < POPULATN_SIZE - 1; i++)
	{
		if(rand_float() < CROSSOVR_RATE)
		{
			point = rand_int(0, string_length);

			for(unsigned int j = 0; j < point; j++)
			{
				_pop[i]->text[j] = _parents[i]->text[j];
				_pop[i+1]->text[j] = _parents[i+1]->text[j];
			}

			for(unsigned int j = point; j < string_length; j++)
			{
				_pop[i]->text[j] = _parents[i]->text[j];
				_pop[i+1]->text[j] = _parents[i+1]->text[j];
			}
		}
		else
		{
			_pop[i]->text = _parents[i]->text;
			_pop[i]->fitness = _parents[i]->fitness;

			_pop[i+1]->text = _parents[i+1]->text;
			_pop[i+1]->fitness = _parents[i+1]->fitness;
		}
	}
}

void mutation(std::vector<solution*>& _pop)
{
	unsigned int i;

	for(solution * _sol : _pop)
	{
		for(i = 0; i < string_length; i++)
		{
			if(rand_float() < MUTATION_RATE)
			{
				_sol->text[i] = rand_char();
			}
		}

		_sol->fitness = calc_fitness(_sol->text);
	}
}

void copy_solution(solution* src, solution* dest)
{
	dest->text = src->text;
	dest->fitness = src->fitness;
}

void free_memory(std::vector<solution*>& _parents, std::vector<solution*>& _pop)
{
	unsigned int i;

	for(i = 0; i < _parents.size(); i++)
	{
		delete _parents[i];
	}

	for(i = 0; i < _pop.size(); i++)
	{
		delete _pop[i];
	}

	_parents.clear();
	_pop.clear();
}

bool file_exists(const std::string &file_name)
{
	struct stat _stat;
	return stat(file_name.c_str(), &_stat) == 0;
}

int file_size(FILE* file)
{
	int size;

	fseek(file, 0L, SEEK_END);
	size = ftell(file);
	rewind(file);

	return size;
}

FILE* ga_fopen(const std::string& file_name)
{
	FILE* _f;

	_f = fopen(file_name.c_str(), "a");

	if(_f == NULL)
	{
		std::cout << "[ERROR] - Could not create file." << std::endl;
		exit(EXIT_FAILURE);
	}

	if(file_size(_f) < 1)
	{
		fprintf(_f, "GenSize,PopSize,MutationRate,CrossovrRate,StrTarget,Time,Worked\n");
	}

	return _f;
}

void ga_touch(const std::string &path_name)
{
	int fd;
	int rc;

	fd = open(path_name.c_str(), O_WRONLY|O_CREAT|O_NOCTTY|O_NONBLOCK, 0666);

	if(fd == -1)
    {
		std::cout << "[ERROR] - Could not open file." << std::endl;
		exit(EXIT_FAILURE);
    }

	rc = utimensat(AT_FDCWD, path_name.c_str(), NULL, 0);

    if(rc)
    {
		std::cout << "[ERROR] - Could not utimensat()." << std::endl;
        exit(EXIT_FAILURE);
    }
}

void print_usage()
{
	std::cout << "Usage: ./program [options] -w [string]" << "\n\n"
			  << "\t -c \t Crossover rate.\n"
			  << "\t -e \t Enable elitism.\n"
			  << "\t -g \t Generation size.\n"
			  << "\t -i \t Input from stdin.\n"
			  << "\t -m \t Mutation rate.\n"
			  << "\t -o \t Enable result output.\n"
			  << "\t -p \t Population size.\n"
			  << "\t -w \t Word to find.\n"
			  << "\t -v \t Verbose mode.\n"
			  << std::endl;
}

void verify_values()
{
	if(MUTATION_RATE == 0.02f	&&
	   CROSSOVR_RATE == 0.7f	&&
	   POPULATN_SIZE == 1000	&&
	   GENERATN_SIZE == 100000  &&
	   string_length <  64      &&
	   string_length >  0)
	{
		std::cout << "[INFO] - Using default values." << std::endl;
		return;
	}

	// verify values
	if(MUTATION_RATE > 1 || MUTATION_RATE < 0)
	{
		std::cout << "[ERROR] - Mutation rate must be between 0 and 1."
				  << std::endl;
		exit(EXIT_FAILURE);
	}

	if(CROSSOVR_RATE > 1 || CROSSOVR_RATE < 0)
	{
		std::cout << "[ERROR] - Crossover rate must be between 0 and 1."
				  << std::endl;
		exit(EXIT_FAILURE);
	}

	if(POPULATN_SIZE < 1 || POPULATN_SIZE > INT_MAX)
	{
		std::cout << "[ERROR] - Population size must be greater than 1."
				  << std::endl;
		exit(EXIT_FAILURE);
	}

	if(GENERATN_SIZE < 1 || GENERATN_SIZE > INT_MAX)
	{
		std::cout << "[ERROR] - Generation size must be greater than 1."
				  << std::endl;
		exit(EXIT_FAILURE);
	}

	if(string_length > 64 || string_length <= 0)
	{
		std::cout << "[ERROR] - String size is invalid!" << std::endl;
		print_usage();
		exit(EXIT_FAILURE);
	}

	std::cout << "[INFO] - Mutation Rate: "   << MUTATION_RATE << "\n"
			  << "[INFO] - CrossOver Rate: "  << CROSSOVR_RATE << "\n"
			  << "[INFO] - Population Size: " << POPULATN_SIZE << "\n"
			  << "[INFO] - Generation Size: " << GENERATN_SIZE << "\n"
			  << "[INFO] - String: " << string_target << "\n"
			  << "[INFO] - Length: " << string_length << "\n"
			  << std::endl;
}

int main(int argc, char **argv)
{
	int copts;

	bool res;

	while((copts = getopt(argc, argv, "c:eg:him:op:vw:")) != -1)
	{
		switch(copts)
		{
		case 'c':
			CROSSOVR_RATE = atof(optarg);
			break;
		case 'e':
			elitism_flag = true;
			break;
		case 'g':
			GENERATN_SIZE = atoi(optarg);
			break;
		case 'i':
			std::cout << "Crossover Rate: ";
			std::cin >> CROSSOVR_RATE;
			std::cout << "Generation Size: ";
			std::cin >> GENERATN_SIZE;
			std::cout << "Mutation Rate: ";
			std::cin >> MUTATION_RATE;
			std::cout << "Population Size: ";
			std::cin >> POPULATN_SIZE;
			break;
		case 'm':
			MUTATION_RATE = atof(optarg);
			break;
		case 'o':
			output_flag = true;
			break;
		case 'p':
			POPULATN_SIZE = atoi(optarg);
			break;
		case 'v':
			verbose_flag = true;
			break;
		case 'w':
			string_target = optarg;
			string_length = string_target.length();
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}

	verify_values();

	if(output_flag)
	{
		res = file_exists(FILE_NAME);

		if(!res)
		{
			ga_touch(FILE_NAME);
		}
	}

	clock_t time_begin;
	clock_t time_end;

	double time_elapsed;

	std::string first_pop;
	std::string last_pop;

	unsigned int max_fitness;
	unsigned int max_fitness_hold;

	srand(time(NULL));

	FILE* f_csv;

	solution* best_sol;
	solution* elitism_solution(new solution("", 0));

	std::vector<solution*> parents;
	std::vector<solution*> population;

	time_begin = clock();

	f_csv = ga_fopen(FILE_NAME);

	parents_init(parents);
	population_init(population);

	first_pop = population[0]->text;

	for(int i = 0; i < GENERATN_SIZE; i++)
	{
		if(i == 0)
		{
			max_fitness = get_max_fitness(population);
		}

		best_sol = get_best_solution(population);

		if(elitism_flag)
		{
			copy_solution(best_sol, elitism_solution);
			copy_solution(elitism_solution, population[0]);
		}

		// operators for population
		tournament(population, parents);
		crossover(population, parents);
		mutation(population);

		max_fitness_hold = get_max_fitness(population);

		if(max_fitness < max_fitness_hold)
		{
			max_fitness = max_fitness_hold;
			last_pop = get_best_solution_text(population);
		}

		if(verbose_flag)
			std::cout << "[INFO] - Gen: "
					  << i
					  << std::endl;

		print_sol_max_fitness(population);

		if(verbose_flag)
			std::cout << "[INFO] - Max fitness: "
					  << max_fitness
					  << std::endl;

		if(max_fitness == string_length)
		{
			last_pop = get_best_solution_text(population);
			break;
		}
	}

	time_end = clock();

	time_elapsed = (double)(time_end - time_begin) / CLOCKS_PER_SEC;

	std::cout << "\n[INFO] - From \""
			  << first_pop
			  << "\" to \""
			  << last_pop
			  << "\" in "
			  << time_elapsed
			  << " seconds."
			  << std::endl;

	if(string_target == last_pop) worked_flag = true;

	if(output_flag)
	{
		fprintf(f_csv, "%d,%d,%0.2f,%0.2f,%s,%0.4f,%d\n", GENERATN_SIZE,
				POPULATN_SIZE, MUTATION_RATE,
				CROSSOVR_RATE , string_target.c_str(), time_elapsed, (worked_flag ? 1 : 0));
	}

	fclose(f_csv);
	delete elitism_solution;
	free_memory(parents, population);

    return 0;
}
