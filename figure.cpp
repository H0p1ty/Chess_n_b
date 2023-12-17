#include "figure.h"
#include "board.h"
#include <algorithm>
#include <iostream>
#include <typeinfo>
#include <vector>

using Graph_lib::Circle;

bool king_is_under_attack (Chessboard& chess, bool is_white)
{
    Cell* king_ptr = nullptr;
    for (int i = a_ascii; i < chess.N + a_ascii; i++)
        for (int j = 1; j <= chess.N; j++)
            if (chess[i][j].has_figure() && chess[i][j].get_figure().is_king() &&
                chess[i][j].get_figure().is_white() == is_white)
                king_ptr = &(chess[i][j]);
    if (king_ptr == nullptr)
        throw std::runtime_error("No king!");
    for (int i = a_ascii; i < a_ascii + chess.N; i++)
        for (int j = 1; j <= chess.N; j++)
            if (chess[i][j].has_figure() && chess[i][j].get_figure().is_white() != is_white &&
                chess[i][j].get_figure().can_take_king(chess, *king_ptr))
                return true;
    return false;
}

void Figure::attach(const Cell& c)
{
    move(c.center().x - point(0).x - 40, c.center().y - point(0).y - 45);
    cell = &c;
}

bool Figure::change_pos_decider(Cell& c)
{
    if (c.has_figure())
    {
        if (is_white())
        {
            if (c.get_figure().is_white())
                return false;
        }
        else if (is_black())
        {
            if (c.get_figure().is_black())
                return false;
        }
    }
    return true;
}

void Pawn::reset_double_step ()
{
    if(steps_till_reset == 0)
        double_step = 0;
    else
        steps_till_reset--;
}

int Pawn::correct_step(Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe)
{
    int decider;  // Decides whether to move upwards or downwards
                  // depending on the color of a figure.

    // Decides whether to take the figure or not
    bool take_decider;  // If true, it assumes that it's whites' turn
                        // else - blacks' turn.
    int returning_value = 0;

    bool first_step_reserved = first_step;
    bool double_step_reserved = double_step;
    int steps_till_reset_reserved = steps_till_reset;

    if (is_white())
        decider = 1;  // For some reason ternary operator didn't work so
    else              // I had to set 'decider' value the old-fashioned way
        decider = -1;

    if (decider == 1)
        take_decider = true;
    else
        take_decider = false;

    int x1 = int(c1.location().x);
    int y1 = c1.location().y;
    int x2 = int(c2.location().x);
    int y2 = c2.location().y;

    if ((a_ascii <= x1 - 1 && x1 - 1 < a_ascii + chess.N) &&
        chess[x1 - 1][y1].has_figure() && chess[x1 - 1][y1].get_figure().is_pawn() &&
        chess[x1][y1].get_figure().is_black() != chess[x1 - 1][y1].get_figure().is_black() &&
        chess[x1 - 1][y1].get_figure().double_step0() && (x2 == x1 - 1) && (y2 == y1 + decider))
    {
        first_step = false;
        returning_value = 2;
    }
    else if ((a_ascii <= x1 + 1 && x1 + 1 < a_ascii + chess.N) &&
             chess[x1 + 1][y1].has_figure() && chess[x1 + 1][y1].get_figure().is_pawn() &&
             chess[x1][y1].get_figure().is_black() != chess[x1 + 1][y1].get_figure().is_black() &&
             chess[x1 + 1][y1].get_figure().double_step0() && (x2 == x1 + 1) && (y2 == y1 + decider))
    {
        first_step = false;
        returning_value = 3;
    }
    else if (first_step)
    {
        if (x1 == x2 && ((y2 - y1 == 1 * decider) || (y2 - y1 == 2 * decider)) && !c2.has_figure())
        {
            if (y2 - y1 == 2 * decider)
            {
                if(chess[x1][y1 + decider].has_figure())
                {
                    return 0;
                }
                double_step = true;
                steps_till_reset = 1;
            }
            first_step = false;
            returning_value = 1;
        }

        else if ((x1 == x2 + 1 || x1 == x2 - 1) && (y2 - y1 == 1 * decider) && c2.has_figure() &&
                 c2.get_figure().is_black() == take_decider)
        {
            first_step = false;
            double_step = false;
            returning_value = 1;
        }
        else
            return 0;
    }
    else
    {
        if (x1 == x2 && (y2 - y1 == 1 * decider) && !c2.has_figure())
        {
            double_step = false;
            returning_value = 1;
        }
        else if ((x1 == x2 + 1 || x1 == x2 - 1) && (y2 - y1 == 1 * decider) && c2.has_figure() &&
                 c2.get_figure().is_black() == take_decider)
        {
            double_step = false;
            returning_value = 1;
        }
        else
            return 0;
    }
    if (ensure_king_is_safe == false)
        return returning_value;
    else
    {
        bool has_deleted_figure = false;
        Figure* tmp = nullptr;
        if (c2.has_figure())
        {
            tmp = &(c2.get_figure());
            chess.detach(c2.detach_figure());
            has_deleted_figure = true;
        }
        c2.attach_figure(c1.detach_figure());
        if (king_is_under_attack(chess, c2.get_figure().is_white()))
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }

            first_step = first_step_reserved;
            double_step = double_step_reserved;
            steps_till_reset = steps_till_reset_reserved;

            return -1;
        }
        else
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }
            return returning_value;
        }
    }
    return returning_value;
}

bool Pawn::can_take_king (Chessboard& chess, Cell& king_position)
{
    bool first_step_reserved = first_step;
    bool double_step_reserved = double_step;
    int steps_till_reset_reserved = steps_till_reset;

    bool returning_value = correct_step(*(const_cast<Cell*>(get_cell())), king_position, chess, false);

    first_step = first_step_reserved;
    double_step = double_step_reserved;
    steps_till_reset = steps_till_reset_reserved;
    return (returning_value > 0);
}

VisualSteps* Pawn::show_possible_steps(Coordinate position, Chessboard& chess)  // moves on the board
{
    VisualSteps* steps_representation = new VisualSteps;

    int decider;

    char x = position.x;
    int y = position.y;

    if (is_white())
        decider = 1;
    else
        decider = -1;

    if (position.y == int(4.5 + 3.5 * decider))  // If pawn has reached the end of the board
        return steps_representation;             // It can't move anymore (for now)

    bool first_step_reserved = first_step;
    bool double_step_reserved = double_step;
    int steps_till_reset_reserved = steps_till_reset;

    for (int i = 1; i <= (first_step ? 2 : 1); i++)
    {
        int a = correct_step(chess[x][y], chess[x][y + i * decider], chess);
        if (a > 0)
        {

            first_step = first_step_reserved;
            double_step = double_step_reserved;
            steps_till_reset = steps_till_reset_reserved;

            Circle* tempc = new Circle{chess[x][y + i * decider].center(), c_size / 4};
            AttachManager<Circle> am_tempc{tempc, chess};
            am_tempc.set_color(chess_yellow);
            am_tempc.set_fill_color(chess_yellow);
            steps_representation->possible_steps.push_back(am_tempc);
        }
        else if (a == -1)
        {
            first_step = first_step_reserved;
            double_step = double_step_reserved;
            steps_till_reset = steps_till_reset_reserved;

            RedCross* temprc = new RedCross{chess[x][y + i * decider].center(), chess};
            AttachManager<RedCross> am_temprc{temprc, chess};
            steps_representation->disabled_steps.push_back(am_temprc);
        }
    }
    for (int i = -1; i <= 1; i += 2)
    {
        if (x + i >= a_ascii && x + i < chess.N + a_ascii)
        {
            int a = correct_step(chess[x][y], chess[x + i][y + decider*1], chess);
            if (a > 0)
            {
                first_step = first_step_reserved;
                double_step = double_step_reserved;
                steps_till_reset = steps_till_reset_reserved;

                Frame* tempf{nullptr};
                if (a == 1)
                    tempf = new Frame{chess[x + i][y + decider].center(), chess};
                if (a == 2)
                    tempf = new Frame{chess[x - 1][y + decider].center(), chess};
                if (a == 3)
                    tempf = new Frame{chess[x + 1][y + decider].center(), chess};
                AttachManager<Frame> am_tempf{tempf, chess};
                steps_representation->possible_takes.push_back(am_tempf);
            }
            if (a == -1)
            {
                first_step = first_step_reserved;
                double_step = double_step_reserved;
                steps_till_reset = steps_till_reset_reserved;

                RedCross* temprc = new RedCross{chess[x + i][y + decider*1].center(), chess};
                AttachManager<RedCross> am_temprc{temprc, chess};
                steps_representation->disabled_steps.push_back(am_temprc);
            }
        }
    }
    return steps_representation;
}

int Rook::correct_step(Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe)
{
    int x1 = int(c1.location().x);
    int y1 = c1.location().y;
    int x2 = int(c2.location().x);
    int y2 = c2.location().y;

    bool can_do_castling_reserved = can_do_castling;

    if ((x1 != x2) && (y1 != y2))
        return false;
    if ((x1 == x2) && (y1 == y2))
        return false;

    int d;  // d - decider, used to reduce copy + paste

    if ((x1 != x2) && (y1 == y2))
    {
        d = (x2 > x1) ? 1 : -1;

        for (int i = x1 + d; i != x2; i += d)
            if (chess[i][y1].has_figure())
                return false;
        if (change_pos_decider(c2) == false)
            return false;
    }
    if ((x1 == x2) && (y1 != y2))
    {
        d = (y2 > y1) ? 1 : -1;

        for (int i = y1 + d; i != y2; i += d)
            if (chess[x1][i].has_figure())
                return false;
        if (change_pos_decider(c2) == false)
            return false;
    }
    if (ensure_king_is_safe == false)
    {
        can_do_castling = false;

        return true;
    }
    else
    {
        bool has_deleted_figure = false;
        Figure* tmp = nullptr;
        if (c2.has_figure())
        {
            tmp = &(c2.get_figure());
            chess.detach(c2.detach_figure());
            has_deleted_figure = true;
        }
        c2.attach_figure(c1.detach_figure());
        if (king_is_under_attack(chess, c2.get_figure().is_white()))
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }

            can_do_castling = can_do_castling_reserved;

            return -1;
        }
        else
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }
            can_do_castling = false;

            return true;
        }
    }
}

VisualSteps* Rook::show_possible_steps(Coordinate position, Chessboard& chess)
{
    VisualSteps* steps_representation = new VisualSteps;

    horisontal_possible_steps(position, chess, steps_representation);
    vertical_possible_steps(position, chess, steps_representation);

    return steps_representation;
}

void Rook::horisontal_possible_steps(Coordinate& position, Chessboard& chess, VisualSteps*& steps_representation)
{

    bool can_do_castling_reserved = can_do_castling;

    // d is made to reduce copy + paste
    for (int d = -1; d <= 1; d += 2)
    {
        for (int i = position.x + d; i != a_ascii - 1 + (4.5 + 4.5 * d); i += d)
        {
            int a = correct_step(chess[position.x][position.y], chess[i][position.y], chess);

            can_do_castling = can_do_castling_reserved;
            if (a > 0)
            {
                if (chess[i][position.y].has_figure())
                {
                    Frame* tempf = new Frame{chess[i][position.y].center(), chess};
                    AttachManager<Frame> am_tempf{tempf, chess};
                    steps_representation->possible_takes.push_back(am_tempf);
                    break;
                }
                else
                {
                    Circle* tempc = new Circle{chess[i][position.y].center(), c_size / 4};
                    AttachManager<Circle> am_tempc{tempc, chess};
                    am_tempc.set_color(chess_yellow);
                    am_tempc.set_fill_color(chess_yellow);
                    steps_representation->possible_steps.push_back(am_tempc);
                }
            }
            else if(a == -1)
            {
                RedCross* temprc = new RedCross{chess[i][position.y].center(), chess};
                AttachManager<RedCross> am_temprc{temprc, chess};
                steps_representation->disabled_steps.push_back(am_temprc);
            }
        }
    }
}

void Rook::vertical_possible_steps(Coordinate& position, Chessboard& chess, VisualSteps*& steps_representation)
{

    bool can_do_castling_reserved = can_do_castling;

    // d is made to reduce copy + paste
    for (int d = -1; d <= 1; d += 2)
    {
        for (int i = position.y + d; i != int(4.5 + 4.5 * d); i += d)
        {
            int a = correct_step(chess[position.x][position.y], chess[position.x][i], chess);

            can_do_castling = can_do_castling_reserved;
            if (a > 0)
            {
                if (chess[position.x][i].has_figure())
                {
                    Frame* tempf = new Frame{chess[position.x][i].center(), chess};
                    AttachManager<Frame> am_tempf{tempf, chess};
                    steps_representation->possible_takes.push_back(am_tempf);
                    // delete tempf;
                    break;
                }
                else
                {
                    Circle* tempc = new Circle{chess[position.x][i].center(), c_size / 4};
                    AttachManager<Circle> am_tempc{tempc, chess};
                    am_tempc.set_color(chess_yellow);
                    am_tempc.set_fill_color(chess_yellow);
                    steps_representation->possible_steps.push_back(am_tempc);
                    // delete tempc;
                }
            }
            else if (a == -1)
            {
                RedCross* temprc = new RedCross{chess[position.x][i].center(), chess};
                AttachManager<RedCross> am_temprc{temprc, chess};
                steps_representation->disabled_steps.push_back(am_temprc);
            }
        }
    }
}

int Knight::correct_step(Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe)
{
    int x1 = int(c1.location().x);
    int y1 = c1.location().y;
    int x2 = int(c2.location().x);
    int y2 = c2.location().y;
    if (!((std::abs(x2 - x1) == 1 && std::abs(y2 - y1) == 2) || (std::abs(y2 - y1) == 1 && std::abs(x2 - x1) == 2)))
        return false;

    if ((std::abs(x2 - x1) == 1) && (std::abs(y2 - y1) == 2))
    {
        if (c2.has_figure())
        {
            if (change_pos_decider(c2) == false)
                return false;
        }
    }
    else if ((std::abs(x2 - x1) == 2) && (std::abs(y2 - y1) == 1))
    {
        if (c2.has_figure())
        {
            if (change_pos_decider(c2) == false)
                return false;
        }
    }
    if (ensure_king_is_safe == false)
        return true;
    else
    {
        bool has_deleted_figure = false;
        Figure* tmp = nullptr;
        if (c2.has_figure())
        {
            tmp = &(c2.get_figure());
            chess.detach(c2.detach_figure());
            has_deleted_figure = true;
        }
        c2.attach_figure(c1.detach_figure());
        if (king_is_under_attack(chess, c2.get_figure().is_white()))
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }
            return -1;
        }
        else
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }
            return true;
        }
    }
    return true;
}

VisualSteps* Knight::show_possible_steps(Coordinate position, Chessboard& chess)
{

    VisualSteps* steps_representation = new VisualSteps;

    int x = int(position.x);
    int y = position.y;

    std::vector<Coordinate> potential_steps;

    for (int i = -1; i <= 1; i += 2)
        for (int j = -2; j <= 2; j += 4)
            potential_steps.push_back(Coordinate{char(x + i), y + j});
    for (int i = -2; i <= 2; i += 4)
        for (int j = -1; j <= 1; j += 2)
            potential_steps.push_back(Coordinate{char(x + i), y + j});

    for (const Coordinate& pos : potential_steps)
    {
        if (!chess.out_of_range(pos))
        {
            int a = correct_step(chess[x][y], chess[pos.x][pos.y], chess);
            if (a > 0)
            {
                if (chess[pos.x][pos.y].has_figure())
                {
                    Frame* tempf = new Frame{chess[pos.x][pos.y].center(), chess};
                    AttachManager<Frame> am_tempf{tempf, chess};
                    steps_representation->possible_takes.push_back(am_tempf);
                }
                else
                {
                    Circle* tempc = new Circle{chess[pos.x][pos.y].center(), c_size / 4};
                    AttachManager<Circle> am_tempc{tempc, chess};
                    am_tempc.set_color(chess_yellow);
                    am_tempc.set_fill_color(chess_yellow);
                    steps_representation->possible_steps.push_back(am_tempc);
                }
            }
            else if (a == -1)
            {
                RedCross* temprc = new RedCross{chess[pos.x][pos.y].center(), chess};
                AttachManager<RedCross> am_temprc{temprc, chess};
                steps_representation->disabled_steps.push_back(am_temprc);
            }
        }
    }
    return steps_representation;
}

int Bishop::correct_step(Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe)
{
    int x1 = int(c1.location().x);
    int y1 = c1.location().y;
    int x2 = int(c2.location().x);
    int y2 = c2.location().y;

    if (std::abs(x2 - x1) != std::abs(y2 - y1))
        return false;
    else if (x2 == x1 && y2 == y1)
        return false;

    int d1, d2;  // d1, d2 - decider1, decider2, created to reduce copy + paste

    d1 = (x2 > x1) ? 1 : -1;
    d2 = (y2 > y1) ? 1 : -1;

    int x = x1 + d1;
    int y = y1 + d2;

    while (!chess.out_of_range(Coordinate{char(x), y}) && x != x2 && y != y2)
    {
        if (chess[char(x)][y].has_figure())
            return false;
        x += d1;
        y += d2;
    }
    if (change_pos_decider(c2) == false)
        return false;
    if (ensure_king_is_safe == false)
        return true;
    else
    {
        bool has_deleted_figure = false;
        Figure* tmp = nullptr;
        if (c2.has_figure())
        {
            tmp = &(c2.get_figure());
            chess.detach(c2.detach_figure());
            has_deleted_figure = true;
        }
        c2.attach_figure(c1.detach_figure());
        if (king_is_under_attack(chess, c2.get_figure().is_white()))
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }
            return -1;
        }
        else
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }
            return true;
        }
    }
    return true;
}

VisualSteps* Bishop::show_possible_steps(Coordinate position, Chessboard& chess)
{
    VisualSteps* steps_representation = new VisualSteps;

    int x0 = int(position.x);
    int y0 = position.y;

    int x, y;

    // d1, d2 - decider1, decider2 - those are used to reduce copy + paste
    for (int d1 = -1; d1 <= 1; d1 += 2)
    {
        for (int d2 = -1; d2 <= 1; d2 += 2)
        {
            x = int(position.x) + d1;
            y = position.y + d2;

            show_possible_steps_HF(x, y, x0, y0, d1, d2, steps_representation, chess);
        }
    }

    return steps_representation;
}

void Bishop::show_possible_steps_HF(int x, int y, int x0, int y0, int d1, int d2, VisualSteps*& steps_representation,
                                    Chessboard& chess)
{
    while (!chess.out_of_range(Coordinate{char(x), y}))
    {
        int a = correct_step(chess[x0][y0], chess[x][y], chess);
        if (a > 0)
        {
            if (chess[x][y].has_figure())
            {
                Frame* tempf = new Frame{chess[x][y].center(), chess};
                AttachManager<Frame> am_tempf{tempf, chess};
                steps_representation->possible_takes.push_back(am_tempf);
                break;
            }
            else
            {
                Circle* tempc = new Circle{chess[x][y].center(), c_size / 4};
                AttachManager<Circle> am_tempc{tempc, chess};
                am_tempc.set_color(chess_yellow);
                am_tempc.set_fill_color(chess_yellow);
                steps_representation->possible_steps.push_back(am_tempc);
            }
        }
        else if (a == -1)
        {
            RedCross* temprc = new RedCross{chess[x][y].center(), chess};
            AttachManager<RedCross> am_temprc{temprc, chess};
            steps_representation->disabled_steps.push_back(am_temprc);
        }
        x += d1;
        y += d2;
    }
}

int Queen::correct_step(Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe)
{
    int x1 = int(c1.location().x);
    int y1 = c1.location().y;
    int x2 = int(c2.location().x);
    int y2 = c2.location().y;

    if (x1 == x2 && y1 == y2)
        return false;

    bool cond1 = (std::abs(x2 - x1) == std::abs(y2 - y1));
    bool cond2 = (x2 == x1 && y2 != y1);
    bool cond3 = (x2 != x1 && y2 == y1);

    if (!cond1 && !cond2 && !cond3)
        return false;

    if (((x1 != x2) && (y1 == y2)) || ((x1 == x2) && (y1 != y2)))
    {
        int d;  // d - decider, used to reduce copy + paste

        if ((x1 != x2) && (y1 == y2))
        {
            d = (x2 > x1) ? 1 : -1;

            for (int i = x1 + d; i != x2; i += d)
                if (chess[i][y1].has_figure())
                    return false;
            if (change_pos_decider(c2) == false)
                return false;
        }
        if ((x1 == x2) && (y1 != y2))
        {
            d = (y2 > y1) ? 1 : -1;

            for (int i = y1 + d; i != y2; i += d)
                if (chess[x1][i].has_figure())
                    return false;
            if (change_pos_decider(c2) == false)
                return false;
        }
    }
    else if (x1 != x2 && y1 != y2)
    {
        int d1, d2;  // d1, d2 - decider1, decider2, created to reduce copy + paste

        d1 = (x2 > x1) ? 1 : -1;
        d2 = (y2 > y1) ? 1 : -1;

        int x = x1 + d1;
        int y = y1 + d2;

        while (!chess.out_of_range(Coordinate{char(x), y}) && x != x2 && y != y2)
        {
            if (chess[char(x)][y].has_figure())
                return false;
            x += d1;
            y += d2;
        }
        if (change_pos_decider(c2) == false)
            return false;
    }
    if (ensure_king_is_safe == false)
        return true;
    else
    {
        bool has_deleted_figure = false;
        Figure* tmp = nullptr;
        if (c2.has_figure())
        {
            tmp = &(c2.get_figure());
            chess.detach(c2.detach_figure());
            has_deleted_figure = true;
        }
        c2.attach_figure(c1.detach_figure());
        if (king_is_under_attack(chess, c2.get_figure().is_white()))
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }
            return -1;
        }
        else
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }
            return true;
        }
    }
    return true;
}

VisualSteps* Queen::show_possible_steps(Coordinate position, Chessboard& chess)
{
    VisualSteps* steps_representation = new VisualSteps;

    int x0 = int(position.x);
    int y0 = position.y;

    horisontal_possible_steps(position, chess, steps_representation);
    vertical_possible_steps(position, chess, steps_representation);
    diagnal_possible_steps(x0, y0, position, chess, steps_representation);

    return steps_representation;
}

void Queen::horisontal_possible_steps(Coordinate& position, Chessboard& chess, VisualSteps*& steps_representation)
{
    // d is made to reduce copy + paste
    for (int d = -1; d <= 1; d += 2)
    {
        for (int i = position.x + d; i != a_ascii - 1 + (4.5 + 4.5 * d); i += d)
        {
            int a = correct_step(chess[position.x][position.y], chess[i][position.y], chess);
            if (a > 0)
            {
                if (chess[i][position.y].has_figure())
                {
                    Frame* tempf = new Frame{chess[i][position.y].center(), chess};
                    AttachManager<Frame> am_tempf{tempf, chess};
                    steps_representation->possible_takes.push_back(am_tempf);
                    break;
                }
                else
                {
                    Circle* tempc = new Circle{chess[i][position.y].center(), c_size / 4};
                    AttachManager<Circle> am_tempc{tempc, chess};
                    am_tempc.set_color(chess_yellow);
                    am_tempc.set_fill_color(chess_yellow);
                    steps_representation->possible_steps.push_back(am_tempc);
                }
            }
            else if(a == -1)
            {
                RedCross* temprc = new RedCross{chess[i][position.y].center(), chess};
                AttachManager<RedCross> am_temprc{temprc, chess};
                steps_representation->disabled_steps.push_back(am_temprc);
            }
        }
    }
}

void Queen::vertical_possible_steps(Coordinate& position, Chessboard& chess, VisualSteps*& steps_representation)
{
    // d is made to reduce copy + paste
    for (int d = -1; d <= 1; d += 2)
    {
        for (int i = position.y + d; i != int(4.5 + 4.5 * d); i += d)
        {
            int a = correct_step(chess[position.x][position.y], chess[position.x][i], chess);
            if (a > 0)
            {
                if (chess[position.x][i].has_figure())
                {
                    Frame* tempf = new Frame{chess[position.x][i].center(), chess};
                    AttachManager<Frame> am_tempf{tempf, chess};
                    steps_representation->possible_takes.push_back(am_tempf);
                    break;
                }
                else
                {
                    Circle* tempc = new Circle{chess[position.x][i].center(), c_size / 4};
                    AttachManager<Circle> am_tempc{tempc, chess};
                    am_tempc.set_color(chess_yellow);
                    am_tempc.set_fill_color(chess_yellow);
                    steps_representation->possible_steps.push_back(am_tempc);
                }
            }
            else if (a == -1)
            {
                RedCross* temprc = new RedCross{chess[position.x][i].center(), chess};
                AttachManager<RedCross> am_temprc{temprc, chess};
                steps_representation->disabled_steps.push_back(am_temprc);
            }
        }
    }
}

void Queen::diagnal_possible_steps(int x0, int y0, Coordinate& position, Chessboard& chess,
                                   VisualSteps*& steps_representation)
{
    // d1, d2 - decider1, decider2 - those are used to reduce copy + paste
    for (int d1 = -1; d1 <= 1; d1 += 2)
    {
        for (int d2 = -1; d2 <= 1; d2 += 2)
        {
            int x = int(position.x) + d1;
            int y = position.y + d2;

            show_possible_steps_HF(x, y, x0, y0, d1, d2, steps_representation, chess);
        }
    }
}

void Queen::show_possible_steps_HF(int x, int y, int x0, int y0, int d1, int d2, VisualSteps*& steps_representation,
                                   Chessboard& chess)
{
    while (!chess.out_of_range(Coordinate{char(x), y}))
    {
        int a = correct_step(chess[x0][y0], chess[x][y], chess);
        if (a > 0)
        {
            if (chess[x][y].has_figure())
            {
                Frame* tempf = new Frame{chess[x][y].center(), chess};
                AttachManager<Frame> am_tempf{tempf, chess};
                steps_representation->possible_takes.push_back(am_tempf);
                break;
            }
            else
            {
                Circle* tempc = new Circle{chess[x][y].center(), c_size / 4};
                AttachManager<Circle> am_tempc{tempc, chess};
                am_tempc.set_color(chess_yellow);
                am_tempc.set_fill_color(chess_yellow);
                steps_representation->possible_steps.push_back(am_tempc);
            }
        }
        else if(a == -1)
        {
            RedCross* temprc = new RedCross{chess[x][y].center(), chess};
            AttachManager<RedCross> am_temprc{temprc, chess};
            steps_representation->disabled_steps.push_back(am_temprc);
        }
        x += d1;
        y += d2;
    }
}

int King::correct_step(Cell& c1, Cell& c2, Chessboard& chess, bool ensure_king_is_safe)
{
    bool dlsc = false; // did_left_short_castling
    bool dllc = false; // did_left_long_castling
    bool drsc = false; // did_right_short_castling
    bool drlc = false; // did_right_long_castling

    bool can_do_castling_reserved = can_do_castling;

    int x1 = int(c1.location().x), x2 = int(c2.location().x);
    int y1 = c1.location().y, y2 = c2.location().y;

    if(can_do_castling)
    {
        for(int d = -1; d <= 1; d += 2) // d - decider, for less copy + paste
        {
            if(a_ascii <= x1+d && x1+d < a_ascii + chess.N && a_ascii <= x1+2*d && x1+2*d < a_ascii + chess.N &&
               x2-x1 == 2*d && y2 == y1 && !chess[x1+d][y1].has_figure() && !chess[x1+2*d][y1].has_figure())
            {
                if(a_ascii <= x1+3*d && x1+3*d < a_ascii + chess.N && chess[x1+3*d][y1].has_figure() &&
                   is_white() == chess[x1+3*d][y1].get_figure().is_white() &&
                   chess[x1+3*d][y1].get_figure().is_rook() && dynamic_cast<Rook*>(&(chess[x1+3*d][y1].get_figure()))->castling())
                {
                    if(d == -1)
                        dlsc = true;
                    else if(d == 1)
                        drsc = true;
                    can_do_castling = false;
                }
                else if(a_ascii <= x1+4*d && x1+4*d < a_ascii + chess.N && a_ascii <= x1+3*d && x1+3*d < a_ascii + chess.N &&
                        chess[x1+4*d][y1].has_figure() && chess[x1+4*d][y1].get_figure().is_rook() &&
                        is_white() == chess[x1+4*d][y1].get_figure().is_white() &&
                        !chess[x1+3*d][y1].has_figure() && dynamic_cast<Rook*>(&(chess[x1+4*d][y1].get_figure()))->castling())
                {
                    if(d == -1)
                        dllc = true;
                    else if(d == 1)
                        drlc = true;
                    can_do_castling = false;
                }
            }
        }
    }
    if ((can_do_castling == can_do_castling_reserved &&
         !(abs(x2 - x1) <= 1 && abs(y2 - y1) <= 1)) || (x2 == x1 && y1 == y2))
        return false;
    if (change_pos_decider(c2) == false)
        return false;

    can_do_castling = false;
    possible_castlings = {dlsc, dllc, drsc, drlc};

    if (ensure_king_is_safe == false)
        return true;

    else
    {
        bool has_deleted_figure = false;
        Figure* tmp = nullptr;
        if (c2.has_figure())
        {
            tmp = &(c2.get_figure());
            chess.detach(c2.detach_figure());
            has_deleted_figure = true;
        }
        c2.attach_figure(c1.detach_figure());

        if (dlsc)
            chess[x1-1][y1].attach_figure(chess[x1-3][y1].detach_figure());
        else if (dllc)
            chess[x1-1][y1].attach_figure(chess[x1-4][y1].detach_figure());
        else if (drsc)
            chess[x1+1][y1].attach_figure(chess[x1+3][y1].detach_figure());
        else if (drlc)
            chess[x1+1][y1].attach_figure(chess[x1+4][y1].detach_figure());

        if (king_is_under_attack(chess, c2.get_figure().is_white()))
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }

            if (dlsc)
                chess[x1-3][y1].attach_figure(chess[x1-1][y1].detach_figure());
            else if (dllc)
                chess[x1-4][y1].attach_figure(chess[x1-1][y1].detach_figure());
            else if( drsc)
                chess[x1+3][y1].attach_figure(chess[x1+1][y1].detach_figure());
            else if (drlc)
                chess[x1+4][y1].attach_figure(chess[x1+1][y1].detach_figure());
            
            can_do_castling = can_do_castling_reserved;

            return -1;
        }
        else
        {
            c1.attach_figure(c2.detach_figure());
            if (has_deleted_figure)
            {
                c2.attach_figure(*tmp);
                chess.attach(*tmp);
            }

            if (dlsc)
                chess[x1-3][y1].attach_figure(chess[x1-1][y1].detach_figure());
            else if (dllc)
                chess[x1-4][y1].attach_figure(chess[x1-1][y1].detach_figure());
            else if( drsc)
                chess[x1+3][y1].attach_figure(chess[x1+1][y1].detach_figure());
            else if (drlc)
                chess[x1+4][y1].attach_figure(chess[x1+1][y1].detach_figure());

            return true;
        }
    }
    return true;
}

VisualSteps* King::show_possible_steps(Coordinate position, Chessboard& chess)
{
    // I know it's less effective, but easier to read, maybe should be rewritten
    VisualSteps* steps_representation = new VisualSteps;

    bool can_do_castling_reserved = can_do_castling;

    for (int i = a_ascii; i < a_ascii + chess.N; i++)
        for (int j = 1; j <= chess.N; j++)
        {
            int a = correct_step(chess[position.x][position.y], chess[i][j], chess);

            can_do_castling = can_do_castling_reserved;

            if (a > 0)
            {
                if (chess[i][j].has_figure())
                {
                    Frame* tempf = new Frame{chess[i][j].center(), chess};
                    AttachManager<Frame> am_tempf{tempf, chess};
                    steps_representation->possible_takes.push_back(am_tempf);
                }
                else
                {
                    Circle* tempc = new Circle{chess[i][j].center(), c_size / 4};
                    AttachManager<Circle> am_tempc{tempc, chess};
                    am_tempc.set_color(chess_yellow);
                    am_tempc.set_fill_color(chess_yellow);
                    steps_representation->possible_steps.push_back(am_tempc);
                }
            }
            else if (a == -1)
            {
                RedCross* temprc = new RedCross{chess[i][j].center(), chess};
                AttachManager<RedCross> am_temprc{temprc, chess};
                steps_representation->disabled_steps.push_back(am_temprc);
            }
        }

    return steps_representation;
}
