#pragma once

#include "Problem.h"

#include <random>
#include <format>

class ProblemGenerator
{
public:
	ProblemGenerator(int minAddition, int maxAddition, int minMultiplication, int maxMultiplication);

	Problem GenerateProblem();
	Problem GenerateAdditionProblem();
	Problem GenerateSubtractionProblem();
	Problem GenerateMultiplicationProblem();
	Problem GenerateDivisionProblem();

private:
	std::random_device m_rd;
	std::mt19937 m_gen{};
	std::uniform_int_distribution<> m_problemDistrib{ 1, 4 };
	std::uniform_int_distribution<> m_additionDistrib{ 1, 10 };
	std::uniform_int_distribution<> m_multiplicationDistrib{ 1, 10 };
};
