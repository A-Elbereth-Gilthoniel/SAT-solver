#include "sat.h"

//===================================================

CDCL::CDCL(std::vector<std::vector<int>>& cnf, int variable_amount)
{
    for (auto& clause : cnf)
        add_clause(clause);

    for (int i = 1; i <= variable_amount; ++ i)
        activity.push_back(0.0);
}

//====================================================

bool CDCL::process_algorithm()
{
    int conflict_clause_index;
    current_level = 0;
    while (true)
    {
        bool result = unit_propagate(conflict_clause_index);

        if (!result)
        {
            if (current_level == 0)
                return false;


            increase_activity(conflict_clause_index);

            std::vector<int> learned_clause = analyze_conflict(conflict_clause_index);
            std::array<int, 2> lvl_uip = compute_backjump_level(learned_clause);
            int back_level = lvl_uip[0];
            int uip = lvl_uip[1];

            add_clause(learned_clause);
            backjump(back_level);

            assign_literal(uip, clauses.size() - 1);
            continue;
        }

        bool all_clauses_are_satisfied = true;
        for (Clause& it : clauses)
        {
            if (!is_satisfied(it.literals))
            {
                all_clauses_are_satisfied = false;
                break;
            }
        }
        if (all_clauses_are_satisfied)
            return true;

        int var = choose_variable();
        if (var == 0)
            return false;

        current_level++;
        assign_literal(var, -1);
    }
    return false;
}
//====================================

void CDCL::add_clause(std::vector<int>& disjunct)
{
    Clause clause;
    if (disjunct.size() >= 2)
    {
        clause = {disjunct, disjunct[0], disjunct[1]};
        watched_map[disjunct[0]].push_back(clauses.size());
        watched_map[disjunct[1]].push_back(clauses.size());
    }
    else
    {
        clause = {disjunct, disjunct[0], 0};
        watched_map[disjunct[0]].push_back(clauses.size());
    }

    clauses.push_back(clause);
}

//====================================

bool CDCL::is_satisfied(const std::vector<int>& clause)
{
    for (int lit : clause)
    {
        if (lit_is_true(lit))
            return true;
    }
    return false;
}

//====================================

int CDCL::choose_variable()
{
    random_counter++;
    if (random_counter >= 10)
    {
        random_counter = 0;
        for (const Clause clause : clauses)
        {
            for (int lit : clause.literals)
            {
                if (!lit_is_assigned(lit))
                {
                    return lit;

                }
            }
        }
        return 0;
    }

    std::vector<int> indices(activity.size());
    for (int i = 0; i < activity.size(); ++i) {
        indices[i] = i;
    }

    std::sort(indices.begin(), indices.end(), [&](int a, int b) {
        return activity[a] > activity[b];
    });

    for (int i = 0; i < indices.size(); i++)
    {
        int var = indices[i] + 1;
        if (!lit_is_assigned(var))
        {
            return var;
        }
    }
    return 0;
}

//=====================================

void CDCL::increase_activity(int conflict_clause_index)
{
    for (int lit : clauses[conflict_clause_index].literals)
    {
        activity[abs(lit)-1] += bump;
    }
    bump /= 0.95;
}

//=====================================

void CDCL::assign_literal(int lit, int reason_clause_index)
{
    var_conditions[abs(lit)] = {lit > 0, current_level, reason_clause_index};
    propagation_queue.push_back(lit);
    condition_stack.push(abs(lit));
    assert(lit != 0);
}

//=====================================

bool CDCL::unit_propagate(int& conflict_clause_index)
{
    bool changed = true;
    while (changed)
    {
        changed = false;
        while (!propagation_queue.empty())
        {
            int lit = propagation_queue.front();
            propagation_queue.pop_front();
            if (!lit_is_assigned(lit))
                continue;
            if (var_conditions[abs(lit)].level > current_level)
                continue;
            assert(lit != 0);
            int counter = 0;
            std::vector<int> copy_list = watched_map[-lit];
            for (int i = 0; i < copy_list.size(); i++)
            {

                int clause_idx = copy_list[i];
                counter++;
                Clause& clause = clauses[clause_idx];
                int other_watch = (clause.watched1 == -lit) ? clause.watched2 : clause.watched1;

                if (other_watch == 0)
                {
                    conflict_clause_index = clause_idx;
                    return false;
                }
                int new_watched = 0;

                bool found_new_watched = false;
                for (int other_lit : clause.literals)
                {
                    if (other_lit != clause.watched1 && other_lit != clause.watched2 && !lit_is_false(other_lit))
                    {
                        if (clause.watched1 == -lit)
                            clause.watched1 = other_lit;
                        else if (clause.watched2 == -lit)
                            clause.watched2 = other_lit;
                        else
                            assert(false);

                        assert(other_lit != 0);
                        watched_map[other_lit].push_back(clause_idx);
                        watched_map[-lit].erase(std::find(watched_map[-lit].begin(), watched_map[-lit].end(), clause_idx));
                        found_new_watched = true;
                        break;
                    }
                }

                if (!found_new_watched)
                {
                    if (lit_is_true(other_watch))
                        continue;

                    if (!lit_is_assigned(other_watch))
                    {
                        assign_literal(other_watch, clause_idx);
                        assert(lit_is_assigned(other_watch));
                        changed = true;

                    }

                    if (lit_is_false(other_watch))
                    {
                        conflict_clause_index = clause_idx;
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

//========================================================

bool CDCL::lit_is_assigned(int lit)
{
    auto it = var_conditions.find(abs(lit));
    return (it != var_conditions.end());
}

//============================================================

bool CDCL::lit_is_false(int lit)
{
    if (!lit_is_assigned(lit))
        return false;
    return !((lit > 0 && var_conditions[abs(lit)].value) || (lit < 0 && !var_conditions[abs(lit)].value));
}

//========================================================

bool CDCL::lit_is_true(int lit)
{
    if (!lit_is_assigned(lit))
        return false;
    return ((lit > 0 && var_conditions[abs(lit)].value) || (lit < 0 && !var_conditions[abs(lit)].value));
}

//========================================================

std::vector<int> CDCL::analyze_conflict(int conflict_clause_ind)
{
    std::vector<int> new_clause;
    std::unordered_set<int> seen_vars, new_clause2;
    std::stack<int> current_level_vars;


    for (int lit : clauses[conflict_clause_ind].literals)
    {
        int var = abs(lit);
        if (var_conditions[var].level == current_level)
            current_level_vars.push(var);
        seen_vars.insert(var);
        new_clause2.insert(var);
    }

    while (!current_level_vars.empty())
    {
        int var = current_level_vars.top();
        current_level_vars.pop();

        if (var_conditions[var].reason_clause_index == -1)
            continue;

        for (int literal : clauses[var_conditions[var].reason_clause_index].literals)
        {
            int new_var = abs(literal);
            if (seen_vars.count(new_var) == 0)
            {
                seen_vars.insert(new_var);
                new_clause2.insert(new_var);
                if (var_conditions[new_var].level == current_level)
                {
                    current_level_vars.push(new_var);
                }
            }
        }
        new_clause2.erase(var);

    }

    for (int var : new_clause2)
    {
        bool val = var_conditions[var].value;
        new_clause.push_back(val ? -var : var);
    }

    return new_clause;
}

//========================================================================

std::array<int, 2> CDCL::compute_backjump_level(const std::vector<int>& learned_clause)
{
    int max_level = -1;
    int second_max_level = -1;
    int uip;

    for (size_t i = 0; i < learned_clause.size(); ++i)
    {
        int var = abs(learned_clause[i]);
        assert(lit_is_assigned(var));
        int level = var_conditions[var].level;
        if (level > max_level)
        {
            uip = learned_clause[i];
            second_max_level = max_level;
            max_level = level;
        }
        else if (level > second_max_level && level != max_level)
        {
            second_max_level = level;
        }
    }

    return {std::max(0, second_max_level), uip};
}

//=============================================================

void CDCL::backjump(int new_level)
{
    while (!propagation_queue.empty() && \
            var_conditions[abs(propagation_queue.back())].level > new_level)
    {
        propagation_queue.pop_back();
    }

    while (!condition_stack.empty())
    {
        int var = condition_stack.top();
        assert(lit_is_assigned(var));
        if (var_conditions[var].level <= new_level)
            break;
        var_conditions.erase(var);
        condition_stack.pop();
    }
    current_level = new_level;
}

// =========================================
