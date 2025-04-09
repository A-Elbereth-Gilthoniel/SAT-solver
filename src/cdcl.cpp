#include "sat.h"



CDCL::CDCL(std::vector<std::vector<int>>& cnf, int variable_amount)
{
    for (auto& clause : cnf)
        add_clause(clause);

    for (int i = 1; i <= variable_amount; ++ i)
        activity.push_back(0.0);
}

//===================================================

void CDCL::add_clause(std::vector<int>& disjunct)
{
#ifdef DEBUG
    std::cout << "New clause : ";
    for (int i = 0; i < disjunct.size(); i++)
    {
        std::cout << disjunct[i] << " ";
        if (lit_is_assigned(disjunct[i]))
            std::cout << "(" << var_conditions[abs(disjunct[i])].level << ") ";            // какого хуя
    }
    std::cout << "index = " << clauses.size() << std::endl;
#endif

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

//
#ifdef DEBUG
void CDCL::print_stack_deque()
{
    std::cout << "queue : ";
        for (const int& val : propagation_queue) {
            assert(lit_is_assigned(val));
            std::cout << val << "(" <<var_conditions[abs(val)].level<<") ";
        }
        std::cout << std::endl << "stack : ";
        auto copy = condition_stack;
        while (!copy.empty()) {
            int val = copy.top();
            copy.pop();
            assert(lit_is_assigned(val));
            std::cout << val << "(" <<var_conditions[val].level<<") ";
        }
}
#endif
//====================================================

bool CDCL::process_algorithm()
{
    int conflict_clause_index;
    current_level = 0;
    // int counter = 0;
    while (true)
    {
        // counter++;
#ifdef DEBUG
        std::cout <<std::endl << std::endl << std::endl<< "Conflict detecting..." << std::endl;

        print_stack_deque();
    std::cout << std::endl;
#endif

        bool result = unit_propagate(conflict_clause_index);
        if (!result)
        {
            if (current_level == 0)
            {
#ifdef DEBUG
                std::cout << "size of cnf is " << clauses.size() << std::endl;
                for (int i = 90; i < clauses.size(); i++)
                {
                    for (int j : clauses[i].literals)
                        std::cout << j << " ";
                    std::cout << std::endl;
                }

                std::cout <<"All Varyables: at end" << std::endl;
            for (auto& it : var_conditions)
            {
                std::cout << it.first << " (" << it.second.value << ") " << std::endl;
            }
#endif
                return false;

            }
            increase_activity(conflict_clause_index);
            std::vector<int> learned_clause = analyze_conflict(conflict_clause_index);
            std::array<int, 2> aa = compute_backjump_level(learned_clause);
            int back_level = aa[0];
            int uip = aa[1];
            add_clause(learned_clause);
            backjump(back_level);
            // std::cout << "uip is " << uip << "("<< back_level<< "). Assigned = " << lit_is_assigned(uip) << std::endl;

            int var = abs(uip);

            // current_level++;
            assert(clauses.size() - 1 >= 0);
            assign_literal(uip, clauses.size() - 1);
#ifdef DEBUG
            if (lit_is_assigned(var))
                std::cout << "uip is " << uip << "("<< back_level<< "). Value = " << (var_conditions[var].value ? 1 : 0) << std::endl;
#endif
            // usleep(1000000);
            // condition_stack.push(var);

            continue;
        }
#ifdef DEBUG
        std::cout << "No conflict" << std::endl;
#endif
        // bool all_clauses_are_satisfied = std::all_of(clauses.begin(), clauses.end(), [&](const Clause& clause) {
        //     return is_satisfied(clause.literals);
        // });
        // if (all_clauses_are_satisfied)
        //     return true;

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
        {
#ifdef DEBUG
            std::cout <<"All Varyables: at end" << std::endl;
            for (auto& it : var_conditions)
            {
                std::cout << it.first << " (" << it.second.value << ") " << std::endl;
            }
#endif
            return false;
            }           // false ?

        current_level++;
        assign_literal(var, -1);
        // condition_stack.push(var);
    }
    return false;
}
//====================================

bool CDCL::is_satisfied(const std::vector<int>& clause)
{
    for (int lit : clause)
    {
        if (lit_is_true(lit))
            return true;
    }
#ifdef DEBUG
    // std::cout << "Govno : ";
    for (int lit : clause)
    {

        std::cout << lit << " ";
        if (lit_is_assigned(lit))
        {
            std::cout << "(" <<var_conditions[abs(lit)].value << ")";
        }
    }
    std::cout << std::endl;
#endif
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
    #ifdef DEBUG
                    std::cout << "Variable is choosen :     " << lit << std::endl;
    #endif
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
#ifdef DEBUG
    std::cout << "New VARYBLE    " << abs(lit) << "    Value = [" << var_conditions[abs(lit)].value <<"]    Level = ["<< var_conditions[abs(lit)].level <<"]    from idx = ["<< var_conditions[abs(lit)].reason_clause_index << "]    is current_lvl [" << current_level <<"]"<< std::endl;
#endif
    propagation_queue.push_back(lit);
    condition_stack.push(abs(lit));
    assert(lit != 0);
}

//=====================================

// ОШибка: в очередь сначала добавилась "-1", потом "1". ПРи этом "-1" перезаписалась данными для "1", и получилось некорректное поведение

bool CDCL::unit_propagate(int& conflict_clause_index)
{
    bool changed = true;
    while (changed)
    {
        changed = false;
        while (!propagation_queue.empty())
        {
#ifdef DEBUG
            print_stack_deque();
            std::cout << std::endl << std::endl;
#endif
            int lit = propagation_queue.front();
            propagation_queue.pop_front();
#ifdef DEBUG
            assert(lit_is_assigned(lit));
            std::cout <<"From queue NUmber " <<lit << "     value = " << var_conditions[abs(lit)].value << std::endl;
#endif
            if (!lit_is_assigned(lit))
                continue;
            if (var_conditions[abs(lit)].level > current_level)
                continue;
            // int lit = propagation_queue.back();
            // propagation_queue.pop_back();
#ifdef DEBUG
            std::cout << std::endl;
            std::cout << "watched_map["<<-lit<<"] = ";
            for (int clause_idx : watched_map[-lit])
                std::cout << clause_idx << " ";
            std::cout<<std::endl;
#endif
            assert(lit != 0);
            int counter = 0;
            std::vector<int> copy_list = watched_map[-lit];
            for (int i = 0; i < copy_list.size(); i++)
            {

                int clause_idx = copy_list[i];
                // usleep(50000);
                counter++;
                Clause& clause = clauses[clause_idx];
#ifdef DEBUG
                std::cout<<std::endl;
                std::cout << "["<<clause_idx<<"]: ";
                print_clause(clause);

                // std::cout << clause_idx << " "<<clause.watched1 <<" "<< clause.watched2 <<" "<< lit << std::endl;

                std::cout << "watch1 = " << clause.watched1 << "    " << "watch2 = " << clause.watched2 << std::endl;
                std::cout<<std::endl;
#endif
                int other_watch = (clause.watched1 == -lit) ? clause.watched2 : clause.watched1;

                if (other_watch == 0)
                {
                    conflict_clause_index = clause_idx;
                    return false;
                }
                // std::cout << other_watch <<" watched"<< std::endl;
                // std::cout << "lit is " << lit_is_true(-lit) << " and value " << lit << " and assigned " << lit_is_assigned(lit) << std::endl;
                // if (lit_is_true(other_watch))
                //     continue;
                int new_watched = 0;

                bool found_new_watched = false;
                for (int other_lit : clause.literals)
                {
                    // std::cout << "1" << std::endl;
                    // std::cout << "for (int other_lit : clause.literals) iteration" << std::endl;
                    if (other_lit != clause.watched1 && other_lit != clause.watched2 && !lit_is_false(other_lit))
                    {
                        if (clause.watched1 == -lit)
                            clause.watched1 = other_lit;
                        else if (clause.watched2 == -lit)
                            clause.watched2 = other_lit;
                        else
                            assert(false);
#ifdef DEBUG
                        std::cout << "replace watched : " << -lit << " <- " << other_lit << std::endl<<std::endl;
#endif
                        assert(other_lit != 0);
                        watched_map[other_lit].push_back(clause_idx);
                        watched_map[-lit].erase(std::find(watched_map[-lit].begin(), watched_map[-lit].end(), clause_idx));
                        found_new_watched = true;


                        // if (lit_is_true(other_lit))
                            break;
                    }
                }
                // std::cout << "2" << std::endl;

                if (!found_new_watched)
                {
                    if (lit_is_true(other_watch)) //|| !lit_is_assigned(other_watch))
                        continue;

                    if (!lit_is_assigned(other_watch))
                    {
                        assign_literal(other_watch, clause_idx);
                        assert(lit_is_assigned(other_watch));
#ifdef DEBUG
                        std::cout << "Unit propagating " << other_watch << " to value = " << var_conditions[abs(other_watch)].value << std::endl;
#endif
                        changed = true;
                        // condition_stack.push(abs(other_watch));

                    }
                        // std::cout << "3" << std::endl;

                    if (lit_is_false(other_watch))
                    {
                        conflict_clause_index = clause_idx;
#ifdef DEBUG
                        std::cout << "Conflict detected. Conflict clause in "<<conflict_clause_index << std::endl;
                        print_clause(clauses[conflict_clause_index]);
                        std::cout << std::endl;
#endif
                        // watched_map[-lit].erase(watched_map[-lit].begin(), watched_map[-lit].begin() + counter);

                        // std::cout << "Watched_map new: "<< counter << std::endl;
                        // for (int i = 0; i < watched_map[-lit].size(); i++)
                        // {
                        //     std::cout << watched_map[-lit][i] << " ";
                        // }
                        // std::cout << std::endl;
                        return false;
                    }
                }
#ifdef DEBUG
                std::cout << "finally: ";
                print_clause(clause);
                std::cout << "NEW   watch1 = " << clause.watched1 << "    " << "watch2 = " << clause.watched2 << std::endl;
#endif
            }
#ifdef DEBUG
            std::cout << "New    Watched_map["<<-lit<<"] ";
            for (int i = 0; i < watched_map[-lit].size(); i++)
            {
                std::cout << watched_map[-lit][i] << " ";
            }
            std::cout << std::endl;
#endif
            // std::cout << "end of cycle" << std::endl;
            // watched_map[-lit].clear();
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

    // std::cout << "hnjkdsnbk" << std::endl;

    for (int lit : clauses[conflict_clause_ind].literals)
    {
        int var = abs(lit);
        if (var_conditions[var].level == current_level)
            current_level_vars.push(var);
        seen_vars.insert(var);
        new_clause2.insert(var);
    }

    // int num_current_level = current_level_vars.size();

    while (!current_level_vars.empty())
    {
        // std::cout << "11" << std::endl;
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
                    // num_current_level++;
                }
            }
        }
        new_clause2.erase(var);


        // num_current_level--;
        // if (num_current_level < 0)     // Сомнительно ПЗДЦ
        //     break;
    }

    for (int var : new_clause2)
    {
        bool val = var_conditions[var].value;
        new_clause.push_back(val ? -var : var);
        // std::cout << (val ? -var : var) << " "; //
    }
    // std::cout <<""<< std::endl;

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
        // std::cout <<"number"<< i << " " << learned_clause[i] << " " << var_conditions[var].level << std::endl;
        if (level > max_level)
        {
            // std::cout << "uip <- " << learned_clause[i] << std::endl;
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
//=============================================


//     //  while (!propagation_queue.empty())
    //     //  {
    //     //     propagation_queue.pop_back();
    //     //     std::cout << propagation_queue.back() << " ("<<var_conditions[abs(propagation_queue.back())].level<<") ";
    //     //  }
    //     //  std::cout << std::endl;
    //     //  std::cout << "from stack ";
    //     //  while (!condition_stack.empty())
    //     //  {
    //     //     std::cout << condition_stack.top() << " ("<<var_conditions[condition_stack.top()].level<<") ";
    //     //     condition_stack.pop();
    //     //  }
    //     //  std::cout << std::endl;
    //     //  assert(nullptr);

void CDCL::backjump(int new_level)
{
    // assert(lit_is_assigned(abs(propagation_queue.back())));
    while (!propagation_queue.empty() && \
            var_conditions[abs(propagation_queue.back())].level > new_level)     //or >=
    {
        //  std::cout << "from queue " << propagation_queue.back() << " ("<<var_conditions[abs(propagation_queue.back())].level<<") ";
        propagation_queue.pop_back();
        // if (!lit_is_assigned(abs(propagation_queue.back())))
        //     std::cout << (propagation_queue.empty() ? "empty" : "no") << std::endl;
        // assert(lit_is_assigned(abs(propagation_queue.back())));
    }

    while (!condition_stack.empty())
    {
        int var = condition_stack.top();
        assert(lit_is_assigned(var));
#ifdef DEBUG
        std::cout <<std::endl<< var << " " << "level = " << var_conditions[var].level << " " << new_level << std::endl;
#endif
        if (var_conditions[var].level <= new_level)
            break;
#ifdef DEBUG
        std::cout << "delete " << var;
#endif
        var_conditions.erase(var);
        condition_stack.pop();
    }
#ifdef DEBUG
    std::cout << std::endl;
#endif
    current_level = new_level;
}

// bool CDCL::unit_propagate(int& conflict_clause_index)
// {
//     bool changed = true;
//     while (changed)
//     {
//         changed = false;
//         for (int i = 0; i < clauses.size(); i++)
//         {
//             size_t indefinite = 0;
//             int unit_lit;
//             bool satisfied = false;
//             for (int lit : clauses[i])
//             {
//                 // std::cout << lit << " ";
//                 auto it = var_conditions.find(abs(lit));
//                 if (it == var_conditions.end())
//                 {
//                     indefinite++;
//                     unit_lit = lit;
//                 }
//                 else if ((lit > 0 && it->second.value) || (lit < 0 && !it->second.value))
//                 {
//                     satisfied = true;
//                     break;
//                 }
//             }
//             // std::cout << (satisfied ? "sat" : "unsat") << std::endl;
//
//             if (satisfied)
//                 continue;
//
//             if (indefinite == 0)
//             {
//                 conflict_clause_index = i;
//                 return false;
//             }
//
//             if (indefinite == 1)
//             {
//                 var_conditions[abs(unit_lit)] = {(unit_lit > 0), current_level, i};
//                 condition_stack.push(abs(unit_lit));
//                 changed = true;
//             }
//
//         }
//     }
//     return true;
// }
//
// =========================================

#ifdef DEBUG
void CDCL::print_clause(Clause& clause)
{
    for (int i : clause.literals)
    {
        std::cout << i << " ";
        if (lit_is_assigned(i))
            std::cout <<" ("<< var_conditions[abs(i)].value << "," << var_conditions[abs(i)].level << ") ";
        else
            std::cout << " (UND) ";
    }
    std::cout << std::endl;
}
#endif
//==========================================
// std::vector<int> CDCL::analyze_conflict(int conflict_clause_ind)
// {
//     std::vector<int> learned_clause;
//     std::unordered_set<int> seen_vars;
//     std::stack<int> current_level_vars;
//     int current_level_counter = 0;
//
//     // std::cout << "hnjkdsnbk" << std::endl;
//
//     for (int lit : clauses[conflict_clause_ind].literals)
//     {
//         int var = abs(lit);
//         assert(lit_is_assigned(var));
//         if (seen_vars.count(var)) continue;
//         if (var_conditions[var].level == current_level)
//         {
//             current_level_vars.push(abs(lit));
//             current_level_counter++;
//         }
//         // else if (var_conditions[var].reason_clause_index != -1)      // ПОДУМАТЬ
//         else
//         {
//             bool val = var_conditions[var].value;
//             learned_clause.push_back(val ? -var : var);
//             // std::cout << "learned_clause <= " << (val ? -var : var) << " 1"<<std::endl;
//             // learned_clause.push_back(-lit);
//         }
//         seen_vars.insert(var);
//     }
//
//     int uip = 0;
//     // std::cout << counter << " " << current_level_var.size() << std::endl;
//     while (!current_level_vars.empty())
//     {
//         int var = current_level_vars.top();
//         current_level_vars.pop();
//         current_level_counter--;
//         assert(lit_is_assigned(var));
//
//         int reason_index = var_conditions[var].reason_clause_index;
//         uip = var;
//         if (reason_index == -1)
//         {
//             // if (var_conditions[var].level != current_level;
//             //     learned_clause.push_back(var_conditions[var].value ? -var : var);
//             continue;
//         }
//
//         const std::vector<int>& reason_clause = clauses[reason_index].literals;
//
//         for (int lit : reason_clause) {
//             int u = abs(lit);
//             // std::cout << "u = " << u << std::endl;
//
//             if (!lit_is_assigned(u))
//             {
//                 while (!propagation_queue.empty())
//                 {
//                     std::cout << propagation_queue.front() << " (" << var_conditions[propagation_queue.front()].level << ") ";
//                     propagation_queue.pop_front();
//                 }
//                 std::cout << std::endl;
//                 assert(lit_is_assigned(u));
//             }
//
//             if (seen_vars.count(u))
//             {
//                 continue;
//             }
//
//             seen_vars.insert(u);
//
//             if (var_conditions[u].level == current_level) {
//                 current_level_vars.push(u);
//                 current_level_counter++;
//             } else {
//                 bool val = var_conditions[u].value;
//                 learned_clause.push_back(val ? -u : u);
//                 // std::cout << "learned_clause <= " << (val ? -u : u) << " 2"<<std::endl;
//                 // learned_clause.push_back(-lit);
//             }
//         }
//
//         if (current_level_counter == 0)
//             break;
//     }
//
//     if (uip != 0)
//     {
//         assert(lit_is_assigned(uip));
//         bool value = var_conditions[uip].value;
//         learned_clause.insert(learned_clause.begin(), value ? -uip : uip);
//     }
//     else
//     {
//         std::cout << "uip is 0" << std::endl;
//     }
//     // std::cout << "learned_clause <= " << (value ? -uip : uip) << " 3. Assign " << lit_is_assigned(uip) <<std::endl;
//
//     return learned_clause;
// }
//=================================================================
// int CDCL::compute_backjump_level(const std::vector<int>& learned_clause)
// {
//     int max_level = 0;
//     int count_at_max = 0;
//
//     for (int lit : learned_clause) {
//         int var = abs(lit);
//         int level = var_conditions[var].level;
//         if (level > max_level) {
//             max_level = level;
//             count_at_max = 1;
//         } else if (level == max_level) {
//             ++count_at_max;
//         }
//     }
//
//     if (max_level == 0)
//         return 0;
//     if (count_at_max == 1)
//         return max_level - 1;
//     return max_level;
// }
//================================================================
// int CDCL::compute_backjump_level(const std::vector<int>& learned_clause)
// {
//     int uip_var = -1;
//
//     for (int lit : learned_clause)
//     {
//         int var = abs(lit);
//         int level = var_conditions[var].level;
//
//         if (level == current_level)
//         {
//             assert(uip_var == -1);
//             uip_var = var;
//         }
//     }
//
//     int max_level = 0;
//     for (int lit : learned_clause) {
//         int var = abs(lit);
//         int level = var_conditions[var].level;
//         if (level < current_level && level > max_level)
//             max_level = level;
//     }
//
//     return max_level;
// }
//============================================================================
