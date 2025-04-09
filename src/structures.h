struct assignment_info {
    bool value;
    int level;
    int reason_clause_index;
};

struct Clause {
    std::vector<int> literals;
    int watched1;
    int watched2;

    Clause() = default;

    Clause& operator=(const Clause& other) {
        if (this != &other) {
            literals = other.literals;
            watched1 = other.watched1;
            watched2 = other.watched2;
        }
        return *this;
    }

};
