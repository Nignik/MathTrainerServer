#include "ProblemGenerator.h"

ProblemGenerator::ProblemGenerator(int minAddition, int maxAddition, int minMultiplication, int maxMultiplication)
	: m_gen{m_rd()},
	m_additionDistrib{minAddition, maxAddition},
	m_multiplicationDistrib{minMultiplication, maxMultiplication}
{
}

Problem ProblemGenerator::GenerateProblem()
{
	ProblemType problemType = static_cast<ProblemType>(m_problemDistrib(m_gen));
	switch (problemType)
	{
		case ProblemType::addition: return GenerateAdditionProblem();
		case ProblemType::subtraction: return GenerateSubtractionProblem();
		case ProblemType::multiplication: return GenerateMultiplicationProblem();
		case ProblemType::division: return GenerateDivisionProblem();
	}

	return Problem("error", "error");
}

Problem ProblemGenerator::GenerateAdditionProblem()
{
	int num1 = m_additionDistrib(m_gen);
	int num2 = m_additionDistrib(m_gen);

	std::string question = std::format("{} + {}", num1, num2);
	std::string answer = std::to_string(num1 + num2);

	return { question, answer };
}

Problem ProblemGenerator::GenerateSubtractionProblem()
{
	int num1 = m_additionDistrib(m_gen);
	int num2 = m_additionDistrib(m_gen);
	if (num2 > num1)
		std::swap(num1, num2);

	std::string question = std::format("{} - {}", num1, num2);
	std::string answer = std::to_string(num1 - num2);
	return { question, answer };
}

Problem ProblemGenerator::GenerateMultiplicationProblem()
{
	int num1 = m_multiplicationDistrib(m_gen);
	int num2 = m_multiplicationDistrib(m_gen);

	std::string question = std::format("{} x {}", num1, num2);
	std::string answer = std::to_string(num1 * num2);

	return { question, answer };
}

Problem ProblemGenerator::GenerateDivisionProblem()
{
	int num1 = m_multiplicationDistrib(m_gen);
	int num2 = m_multiplicationDistrib(m_gen);
	num1 *= num2;

	std::string question = std::format("{} / {}", num1, num2);
	std::string answer = std::to_string(num1 / num2);

	return { question, answer };
}

