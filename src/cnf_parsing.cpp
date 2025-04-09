#include "sat.h"

std::vector<std::vector<int>> read_cnf(const std::string& file_name, size_t& variable_amount, size_t& clause_amount)
{
    std::vector<std::vector<int>> clauses;
    std::ifstream file(file_name);
    std::string line;
    int literal;

    if (!file.is_open())
    {
        std::cerr << "Couldn't open the file. Make sure that it is right path" << std::endl;
        return clauses;
    }

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == 'c' || line[0] == '%' || line[0] == '0')
            continue;

        if (line[0] == 'p')
        {
            std::string aa, bb;
            std::istringstream(line) >> aa >> bb >> variable_amount >> clause_amount;
            // clauses.reserve(clause_amount);
            continue;
        }

        std::vector<int> clause;
        std::istringstream iss(line);
        while (iss >> literal && literal != 0)
        {
            clause.push_back(literal);
        }
        clauses.push_back(clause);

    }

    file.close();

    return clauses;
}
