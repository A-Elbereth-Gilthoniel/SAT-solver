#include "sat.h"

int main(int argc, const char* argv[])
{
#ifdef DEBUG
//     std::ofstream out("log.txt");
//     std::streambuf* coutbuf = std::cout.rdbuf(); // сохранить старый буфер (если нужно восстановить)
//
//     std::cout.rdbuf(out.rdbuf());
#endif

    if (argc != 2)
    {
        std::cerr << "Usage: >>> " << argv[0] << " <input.cnf>" << std::endl;
        return 1;
    }
    size_t variable_amount = 0, clause_amount = 0;

    std::vector<std::vector<int>> clauses = read_cnf(argv[1], variable_amount, clause_amount);
    if (clause_amount == 0)
        return 1;

    CDCL* sat_solver = new CDCL(clauses, variable_amount);
    if (sat_solver->process_algorithm() == true)
        std::cout << "SAT" << std::endl;
    else
        std::cout << "UNSAT" << std::endl;

    return 0;
}
