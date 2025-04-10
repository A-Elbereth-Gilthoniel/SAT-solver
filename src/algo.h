class CDCL {
public:
    CDCL(std::vector<std::vector<int>>& cnf, int variable_amount, int clause_amount);
    bool process_algorithm();
    ~CDCL() = default;

private:
    size_t variable_amount;
    size_t clause_amount;
    std::deque<int> propagation_queue;
    std::vector<Clause> clauses;
    std::vector<std::vector<int>> watched_map;
    std::vector<assignment_info> var_conditions;
    std::stack<int> condition_stack;
    int current_level;
    std::vector<double> activity;
    double bump = 1.0;
    short random_counter = 0;

    void increase_activity(int conflict_clause_index);
    void assign_literal(int lit, int reason_clause_index);
    void add_clause(std::vector<int>& disjunct);
    bool is_satisfied(const std::vector<int>& clause);
    int choose_variable();
    bool unit_propagate(int& conflict_clause_index);
    std::vector<int> analyze_conflict(int conflict_clause_ind);
    std::array<int, 2> compute_backjump_level(const std::vector<int>& learned_clause);
    bool lit_is_true(int lit);
    bool lit_is_false(int lit);
    void backjump(int new_level);
    bool lit_is_assigned(int lit);
};
