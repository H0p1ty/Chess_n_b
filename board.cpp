#include "board.h"
#include "cell.h"
#include <iostream>

Cell& Sub_Vector_ref::operator[](int i)
{
    if(i < 1 || i > 8)
        Graph_lib::error("Out of range: vertical");
    return v[i-1];
}

MyWindow::MyWindow(Point xy, int w, int h, const std::string& title)
    : Simple_window{xy, w, h, title}, quit_button{Point{x_max() - 70, 20}, 70, 20, "Quit", cb_quit}
{
    attach(quit_button);
}

void MyWindow::cb_quit(Address, Address widget)
{
    auto& btn = Graph_lib::reference_to<Graph_lib::Button>(widget);
    dynamic_cast<MyWindow&>(btn.window()).quit();
}

Cell::Type type_of_cell (int i, int j)
{
    if (i % 2 == 0)
        return (j % 2 == 0) ? Cell::dark_green : Cell::white;
    else
        return (j % 2 == 0) ? Cell::white : Cell::dark_green;
}

std::string letters ()
{
    std::string s(Chessboard::N_max, '\0');
    for (size_t i = 0; i < s.size(); ++i)
    {
        s[i] = 'a' + i;
    }
    return s;
}

std::string digits ()
{
    std::string s(Chessboard::N_max, '\0');

    for (size_t i = 0; i < s.size(); ++i)
    {
        s[i] = '1' + i;
    }
    return s;
}

Chessboard::Chessboard(Point xy) : MyWindow{xy, width, height, "Chessboard"}, x_labels{letters()}, y_labels{digits()}
{
    size_range(width, height, width, height);
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            cells.push_back(new Cell{Point{margin + j * Cell::size, margin + (N - 1 - i) * Cell::size}, cb_clicked,
                                     type_of_cell(i, j)});
            attach(cells[cells.size() - 1]);
        }
    }

    for (int i = 0; i < N; ++i)
    {
        // board corners: left bottom
        constexpr Point lb{margin + Cell::size / 2, margin + N * Cell::size + 10};
        // and right bottom
        constexpr Point rb{margin - 10, margin + N * Cell::size - Cell::size / 2};
        x_labels.add(Point{lb.x + i * Cell::size, lb.y});
        y_labels.add(Point{rb.x, rb.y - i * Cell::size});
    }
    attach(x_labels);
    attach(y_labels);

    all_possible_steps = nullptr;

    step_chooser = step_color::white;
}

void Chessboard::standard_fill()
{
// белые пешки
    for (int i = 0; i < 8; i++)
    {
        figures.push_back(std::make_unique<Pawn>(*this, Figure::Type::white));
        at(char(a_ascii + i), 2)
            .attach_figure(*figures[figures.size() - 1]);
    }

    // чёрные пешки
    for (int i = 8; i < 16; i++)
    {
        figures.push_back(std::make_unique<Pawn>(*this, Figure::Type::black));
        at(char(a_ascii + i % 8), 7)
            .attach_figure(*figures[figures.size() - 1]);
    }

    // белые кони

    figures.push_back(std::make_unique<Knight>(*this, Figure::Type::white));
    figures.push_back(std::make_unique<Knight>(*this, Figure::Type::white));

    at('b', 1).attach_figure(*figures[figures.size() - 2]);
    at('g', 1).attach_figure(*figures[figures.size() - 1]);

    // черыне кони
    figures.push_back(std::make_unique<Knight>(*this, Figure::Type::black));
    figures.push_back(std::make_unique<Knight>(*this, Figure::Type::black));

    at('b', 8).attach_figure(*figures[figures.size() - 2]);
    at('g', 8).attach_figure(*figures[figures.size() - 1]);

    // белые слоны
    figures.push_back(std::make_unique<Bishop>(*this, Figure::Type::white));
    figures.push_back(std::make_unique<Bishop>(*this, Figure::Type::white));

    at('c', 1).attach_figure(*figures[figures.size() - 2]);
    at('f', 1).attach_figure(*figures[figures.size() - 1]);

    // черные слоны
    figures.push_back(std::make_unique<Bishop>(*this, Figure::Type::black));
    figures.push_back(std::make_unique<Bishop>(*this, Figure::Type::black));

    at('c', 8).attach_figure(*figures[figures.size() - 2]);
    at('f', 8).attach_figure(*figures[figures.size() - 1]);

    // белые ладьи
    figures.push_back(std::make_unique<Rook>(*this, Figure::Type::white));
    figures.push_back(std::make_unique<Rook>(*this, Figure::Type::white));

    at('a', 1).attach_figure(*figures[figures.size() - 2]);
    at('h', 1).attach_figure(*figures[figures.size() - 1]);

    // черные ладьи
    figures.push_back(std::make_unique<Rook>(*this, Figure::Type::black));
    figures.push_back(std::make_unique<Rook>(*this, Figure::Type::black));

    at('a', 8).attach_figure(*figures[figures.size() - 2]);
    at('h', 8).attach_figure(*figures[figures.size() - 1]);

    // Белый ферзь
    figures.push_back(std::make_unique<Queen>(*this, Figure::Type::white));

    at('d', 1).attach_figure(*figures[figures.size() - 1]);

    // Черный ферзь
    figures.push_back(std::make_unique<Queen>(*this, Figure::Type::black));

    at('d', 8).attach_figure(*figures[figures.size() - 1]);

    // Белый король
    figures.push_back(std::make_unique<King>(*this, Figure::Type::white));

    at('e', 1).attach_figure(*figures[figures.size() - 1]);

    // Черный король
    figures.push_back(std::make_unique<King>(*this, Figure::Type::black));

    at('e', 8).attach_figure(*figures[figures.size() - 1]);
}

void Chessboard::clicked(Cell& c)
{
    // if we haven't selected a figure in the previous step
    if (!selected)
    {
        selected = &c;
        c.activate();  // highlights the cell
        if (decide() == false)
        {
            c.deactivate();
            selected = nullptr;
            return;
        }
        // Create visual representation of moves for current figure
        all_possible_steps = c.get_figure().show_possible_steps(c.location(), *this);
    }
    else
    {
        if (selected->has_figure())
        {
            int a = selected->get_figure().correct_step(*selected, c, *this);
            if (a > 0)
            {
                Cell& c1 = *selected;
                int x = c.location().x, y = c.location().y;

                int b;
                if (step_chooser == black)
                    b = 1;
                else if(step_chooser == white)
                    b = -1;
                if (c.has_figure())
                {
                    // taking the figure from the opponent
                    detach(c.detach_figure());
                    // move the figure to the taken place
                    c.attach_figure(c1.detach_figure());
                }
                else if (1 <= y + b && y + b <= 8 && (*this)[x][y + b].has_figure() && (a == 2 || a == 3))
                {
                    // *this <=> chess
                    detach((*this)[x][y + b].detach_figure());
                    (*this)[x][y].attach_figure(c1.detach_figure());
                }
                else if(c1.get_figure().is_king())
                {
                    King* king_ptr = dynamic_cast<King*>(&c1.get_figure());

                    std::array<bool, 4> temp_arr = king_ptr->castlings();

                    int x1 = c1.location().x;
                    int y1 = c1.location().y;

                    if(temp_arr[0])
                        (*this)[x1-1][y1].attach_figure((*this)[x1-3][y1].detach_figure());
                    else if(temp_arr[1])
                        (*this)[x1-1][y1].attach_figure((*this)[x1-4][y1].detach_figure());
                    else if(temp_arr[2])
                        (*this)[x1+1][y1].attach_figure((*this)[x1+3][y1].detach_figure());
                    else if(temp_arr[3])
                        (*this)[x1+1][y1].attach_figure((*this)[x1+4][y1].detach_figure());

                    c.attach_figure(c1.detach_figure());
                }
                else
                {
                    c.attach_figure(c1.detach_figure());
                }

                step_swap();
                reset_double_steps();

                
                if(!is_check() && !is_mate())
                {
                    if(am_check_sign != nullptr)
                    {
                        delete am_check_sign;
                        am_check_sign = nullptr;
                    }
                }
                if(is_check() && !is_mate())
                {
                    if(am_check_sign == nullptr)
                    {
                        DangerSign* check_sign = new DangerSign{find_king(step_chooser)->center(), *this};
                        am_check_sign = new AttachManager<DangerSign>{check_sign, *this};
                    }
                }
                if(is_check() && is_mate())
                {
                    if(am_check_sign == nullptr)
                    {
                        DangerSign* check_sign = new DangerSign{find_king(step_chooser)->center(), *this};
                        am_check_sign = new AttachManager<DangerSign>{check_sign, *this};
                    }
                    show_checkmate_message();
                    step_chooser = none;
                }
                else if(is_stalemate())
                {
                    show_stalemate_message();
                    step_chooser = none;
                }
            }
        }

        selected->deactivate();

        // Clear the screen from visual representation of possible moves for the current figure
        if (all_possible_steps != nullptr)
        {
            delete all_possible_steps;
            all_possible_steps = nullptr;
        }
        selected = nullptr;
    }
    Fl::redraw();
}

//decides if you can do a move on a clicked cell
bool Chessboard::decide()
{
    if (!selected->has_figure())
        return false;
    else if(step_chooser == step_color::none)
        return false;
    else if (step_chooser == step_color::white && selected->get_figure().is_black())
        return false;
    else if (step_chooser == step_color::black && selected->get_figure().is_white())
        return false;
    return true;
}

void Chessboard::reset_double_steps()
{
    for(int i = 0; i < cells.size(); i++)
        if(cells[i].has_figure() && cells[i].get_figure().is_pawn())
            cells[i].get_figure().reset_double_step();
}

bool Chessboard::out_of_range(Coordinate pos)
{
    if ((int(pos.x) < a_ascii) || (int(pos.x) > a_ascii + 7))
        return true;
    else if ((pos.y < 1) || (pos.y > 8))
        return true;
    return false;
}

Sub_Vector_ref Chessboard::operator[](char c)
{
    if(c < 'a' || c > 'h')
        Graph_lib::error("Out of range: horisontal");
    Graph_lib::Vector_ref<Cell> tempv;
    for(int i = 1; i <= N; i++)
        tempv.push_back(at(c, i));
    Sub_Vector_ref subv{tempv};
    return subv;
}

bool Chessboard::is_check()
{
    Cell* king_ptr = find_king(step_chooser);

    for(int i = a_ascii; i < a_ascii + N; i++)
    {
        for(int j = 1; j <= N; j++)
        {
            if(at(i,j).has_figure() &&
               ((step_chooser == white && at(i,j).get_figure().is_black()) ||
                (step_chooser == black && at(i,j).get_figure().is_white())))
            {
                if(at(i,j).get_figure().can_take_king(*this,*king_ptr))
                    return true;
            }
        }
    }
    return false;
}

bool Chessboard::is_mate()
{

    bool pawn_flag = false;

    bool first_step_reserved{};
    bool double_step_reserved{};
    int steps_till_reset_reserved{};

    bool king_flag = false;
    bool rook_flag = false;

    bool can_do_castling_reserved;

    for(int i = a_ascii; i < a_ascii + N; i++)
    {
        for(int j = 1; j <= N; j++)
        {
            if(at(i,j).has_figure() &&
               ((step_chooser == white && at(i,j).get_figure().is_white()) ||
                (step_chooser == black && at(i,j).get_figure().is_black())))
            {
                for(int i_ = a_ascii; i_ < a_ascii + N; i_++)
                {
                    for(int j_ = 1; j_ <= N; j_++)
                    {
                        if(at(i,j).get_figure().is_pawn())
                        {
                            pawn_flag = true;

                            Pawn* pawn_fr = dynamic_cast<Pawn*>(&(at(i,j).get_figure()));

                            first_step_reserved = pawn_fr->first_step;
                            double_step_reserved = pawn_fr->double_step;
                            steps_till_reset_reserved = pawn_fr->steps_till_reset;
                        }
                        if(at(i,j).get_figure().is_king())
                        {
                            king_flag = true;

                            King* king_fr = dynamic_cast<King*>(&(at(i,j).get_figure()));

                            can_do_castling_reserved = king_fr->can_do_castling;
                        }
                        if(at(i,j).get_figure().is_rook())
                        {
                            rook_flag = true;

                            Rook* rook_fr = dynamic_cast<Rook*>(&(at(i,j).get_figure()));

                            can_do_castling_reserved = rook_fr->can_do_castling;
                        }

                        bool cond = !(i == i_ && j == j_) && at(i,j).get_figure().correct_step(at(i,j), at(i_,j_), *this, true) > 0;

                        if(pawn_flag)
                        {
                            Pawn* pawn_fr = dynamic_cast<Pawn*>(&(at(i,j).get_figure()));

                            pawn_fr->first_step = first_step_reserved;
                            pawn_fr->double_step = double_step_reserved;
                            pawn_fr->steps_till_reset = steps_till_reset_reserved;
                        }
                        else if(king_flag)
                        {
                            King* king_fr = dynamic_cast<King*>(&(at(i,j).get_figure()));

                            king_fr->can_do_castling = can_do_castling_reserved;
                        }
                        else if(rook_flag)
                        {
                            Rook* rook_fr = dynamic_cast<Rook*>(&(at(i,j).get_figure()));

                            rook_fr->can_do_castling = can_do_castling_reserved;
                        }

                        pawn_flag = false;
                        king_flag = false;
                        rook_flag = false;

                        if(cond)
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool Chessboard::is_stalemate()
{
    bool big_statement = true;

    for (int i = 0; i < int(figures.size())-2; ++i) {
        if (figures[i]->has_cell())
            big_statement = false;
    }
    for(int i = int(figures.size())-2; i < int(figures.size()); i++)
        if(!(figures[i]->has_cell()))
            throw std::runtime_error("No king!");

    return ((!is_check() && is_mate()) || big_statement);
}

void Chessboard::show_checkmate_message()
{
    std::string who_won = ((step_chooser == white) ? "BLACKS WON" : "WHITES WON");

    Text* txt1 = new Text{Point{DFTBOF + 4*c_size - int(3*standard_font_size), DFTBOF + 4*c_size - standard_font_size},"CHECKMATE!"};
    Text* txt2 = new Text{Point{DFTBOF + 4*c_size - int(3*standard_font_size), DFTBOF + 4*c_size}, who_won};
    texts.push_back(txt1);
    texts[texts.size()-1].set_font_size(standard_font_size);
    this->attach(texts[texts.size()-1]);
    texts.push_back(txt2);
    texts[texts.size()-1].set_font_size(standard_font_size);
    this->attach(texts[texts.size()-1]);
}

void Chessboard::show_stalemate_message()
{
    Text* txt = new Text{Point{DFTBOF + 4*c_size - int(5.5*standard_font_size), DFTBOF + 4*c_size}, "IT'S A STALEMATE!"};
    texts.push_back(txt);
    texts[texts.size()-1].set_font_size(standard_font_size);
    this->attach(texts[texts.size()-1]);
}

Cell* Chessboard::find_king(step_color color)
{
    Cell* king_ptr = nullptr;

    for(int i = a_ascii; i < a_ascii + N; i++)
    {
        for(int j = 1; j <= N; j++)
        {
            if(at(i,j).has_figure() && at(i,j).get_figure().is_king() &&
               ((color == white) == at(i,j).get_figure().is_white()))
            {
                king_ptr = &(at(i,j));
            }
        }
    }
    if(king_ptr == nullptr)
        throw std::runtime_error("Chessboard::find_king(step_color color) : No king!");
    return king_ptr;
}
