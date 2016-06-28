#include <string.h>
#include <unistd.h>

#include <sys/time.h>

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include <iostream>
#include <vector>

#define MUTATION_RATE 0.02
#define CROSSOVR_RATE 0.7
#define POPULATN_SIZE 1000
#define GENERATN_SIZE 100000

typedef unsigned long long ull;

static const char alphanum[] =
	"0123456789"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"- _";

const unsigned int alphanum_len = strlen(alphanum);

std::string string_target;
unsigned int string_length;

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

	std::cout << "[INFO] - Text: "
			  << max_str
			  << std::endl;
}

void population_print(std::vector<solution*>& _pop)
{
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
	unsigned int i;

	for(i = 0; i < POPULATN_SIZE; i++)
	{
		std::string _rands = generate_string();
		_pop.push_back(create_solution(_rands, calc_fitness(_rands)));
	}
}

void parents_init(std::vector<solution*>& _parents)
{
	unsigned int i;

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

	for(unsigned int i = 0; i < POPULATN_SIZE; i++)
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

	for(unsigned int i = 0; i < POPULATN_SIZE - 1; i++)
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

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		std::cout << "[ERROR] - No arguments provided." << std::endl;
		std::cout << "Usage: ./program [string]" << std::endl;
		exit(EXIT_FAILURE);
	}

	clock_t time_begin;
	clock_t time_end;

	double time_elapsed;

	std::string first_pop;
	std::string last_pop;

	unsigned int max_fitness;
	unsigned int max_fitness_hold;

	srand(time(NULL));

	string_target = argv[1];
	string_length = string_target.length();

	if(string_length > 64)
	{
		std::cout << "[ERROR] - String is too long." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << "[INFO] - AlphaNum Length: " << alphanum_len << std::endl;
	std::cout << "[INFO] - String: " << string_target << std::endl;
	std::cout << "[INFO] - Length: " << string_length << std::endl;
	std::cout << "[INFO] - Generations:" << GENERATN_SIZE << std::endl;
	putchar('\n');

	solution* best_sol;
	solution* elitism_solution(new solution("", 0));
	std::vector<solution*> parents;
	std::vector<solution*> population;

	time_begin = clock();

	parents_init(parents);
	population_init(population);

	first_pop = population[0]->text;

	for(unsigned int i = 0; i < GENERATN_SIZE; i++)
	{
		if(i == 0)
		{
			max_fitness = get_max_fitness(population);
		}

		best_sol = get_best_solution(population);
		copy_solution(best_sol, elitism_solution);
		copy_solution(elitism_solution, population[0]);

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

		std::cout << "[INFO] - Gen: "
				  << i
				  << std::endl;

		print_sol_max_fitness(population);

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

	std::cout << "\n[INFO] - From "
			  << first_pop
			  << " to "
			  << last_pop
			  << " in "
			  << time_elapsed
			  << " seconds."
			  << std::endl;

	delete elitism_solution;
	free_memory(parents, population);

    return 0;
}
