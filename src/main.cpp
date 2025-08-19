#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>

double get_display_refresh_rate()
{
    std::cout << "Refresh rate: ";

    double refresh_rate;
    std::cin >> refresh_rate;

    return refresh_rate;
}

double five_percent_limit(double refresh_rate)
{
    return refresh_rate * 0.95;
}

double reflex_formula_limit(double refresh_rate)
{
    return refresh_rate - (refresh_rate * refresh_rate) / 3600.0;
}

double vrr_formula_limit(double reflex_fps)
{
    return reflex_fps * 0.995;
}

void print_row(std::string label, double value, std::array<int, 4> width)
{
    std::cout << std::setw(width[0]) << (label + " |");
    std::cout << std::setw(width[1] - 2) << static_cast<int>(value) << " |";
    std::cout << std::setw(width[2] - 2) << static_cast<int>(std::round(value)) << " |";
    std::cout << std::setw(width[3] - 2) << value << " |";
    std::cout << '\n';
}

int main()
{
    double refresh_rate = get_display_refresh_rate();
    if (!refresh_rate)
    {
        std::cerr << "Invalid refresh rate." << '\n';
        return 1;
    }

    double percent_fps = five_percent_limit(refresh_rate);
    double reflex_fps = reflex_formula_limit(refresh_rate);
    double optimal_fps = vrr_formula_limit(reflex_fps);

    std::cout << std::fixed << std::setprecision(3) << std::right;

    std::array<int, 4> width = {22, 16, 14, 21};

    std::cout << "\n";
    std::cout << std::setw(width[0]) << "Limit type |";
    std::cout << std::setw(width[1]) << "Truncated FPS |";
    std::cout << std::setw(width[2]) << "Rounded FPS |";
    std::cout << std::setw(width[3]) << "Floating-point FPS |";
    std::cout << '\n';

    std::cout << std::string(width[0] - 1, '-') << "+";
    std::cout << std::string(width[1] - 1, '-') << "+";
    std::cout << std::string(width[2] - 1, '-') << "+";
    std::cout << std::string(width[3] - 1, '-') << "+";
    std::cout << '\n';

    print_row("5% Percent limit", percent_fps, width);
    print_row("Reflex formula limit", reflex_fps, width);
    print_row("Special K VRR limit", optimal_fps, width);

    return 0;
}
