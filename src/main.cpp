#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <dxgi.h>

constexpr std::array<int, 4> column_width{{21, 14, 12, 19}};

double custom_display_refresh_rate(const std::vector<std::string> &argument_vector)
{
    if (argument_vector[1] != "-c")
    {
        return 0.0;
    }
    return std::stod(argument_vector[2]);
}

double get_dxgi_refresh_rate()
{
    IDXGIFactory *pFactory{};
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory)))
    {
        return 0.0;
    }

    IDXGIAdapter *pAdapter{};
    if (FAILED(pFactory->EnumAdapters(0u, &pAdapter)))
    {
        pFactory->Release();
        return 0.0;
    }

    IDXGIOutput *pOutput{};
    if (FAILED(pAdapter->EnumOutputs(0u, &pOutput)))
    {
        pAdapter->Release();
        pFactory->Release();
        return 0.0;
    }

    DXGI_OUTPUT_DESC outputDesc{};
    if (FAILED(pOutput->GetDesc(&outputDesc)))
    {
        pOutput->Release();
        pAdapter->Release();
        pFactory->Release();
        return 0.0;
    }

    DXGI_MODE_DESC targetMode{};
    targetMode.Width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    targetMode.Height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
    targetMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    targetMode.RefreshRate.Numerator = 0u;
    targetMode.RefreshRate.Denominator = 0u;
    targetMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    targetMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_MODE_DESC closestMatch{};
    pOutput->FindClosestMatchingMode(&targetMode, &closestMatch, nullptr);

    pOutput->Release();
    pAdapter->Release();
    pFactory->Release();

    const double numerator{static_cast<double>(closestMatch.RefreshRate.Numerator)};
    const double denominator{static_cast<double>(closestMatch.RefreshRate.Denominator)};

    if (!denominator)
    {
        return 0.0;
    }

    return numerator / denominator;
}

double get_gdi_refresh_rate()
{
    DEVMODE devMode{};
    devMode.dmSize = sizeof(devMode);
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode))
    {
        return static_cast<double>(devMode.dmDisplayFrequency);
    }
    return 0.0;
}

double query_display_refresh_rate()
{
    double refresh_rate{};

    refresh_rate = get_dxgi_refresh_rate();
    if (refresh_rate)
    {
        return refresh_rate;
    }

    refresh_rate = get_gdi_refresh_rate();
    if (refresh_rate)
    {
        return refresh_rate;
    }

    return 0.0;
}

constexpr double rtss_vrr_limit(const double &refresh_rate)
{
    return refresh_rate * 0.95;
}

constexpr double sk_reflex_formula_limit(const double &refresh_rate)
{
    return refresh_rate - (refresh_rate * refresh_rate) / 3600.0;
}

constexpr double sk_vrr_formula_v1_limit(const double &reflex_fps)
{
    return reflex_fps * 0.995;
}

constexpr double sk_vrr_formula_v2_limit(const double &reflex_fps)
{
    return reflex_fps * 0.990;
}

constexpr double sk_vrr_formula_v3_limit(const double &reflex_fps)
{
    return reflex_fps - 0.005 * reflex_fps - 0.0015f;
}

void print_row_title()
{
    std::cout << std::setw(column_width[0]) << "Limit type" << " |";
    std::cout << std::setw(column_width[1]) << "Truncated FPS" << " |";
    std::cout << std::setw(column_width[2]) << "Rounded FPS" << " |";
    std::cout << std::setw(column_width[3]) << "Floating-point FPS" << " |";
    std::cout << '\n';
}

void print_row_separator()
{
    std::cout << std::string(column_width[0] + 1, '-') << "+";
    std::cout << std::string(column_width[1] + 1, '-') << "+";
    std::cout << std::string(column_width[2] + 1, '-') << "+";
    std::cout << std::string(column_width[3] + 1, '-') << "+";
    std::cout << '\n';
}

void print_row_data(const std::string &label, const double &value)
{
    std::cout << std::setw(column_width[0]) << label << " |";
    std::cout << std::setw(column_width[1]) << static_cast<int>(value) << " |";
    std::cout << std::setw(column_width[2]) << static_cast<int>(std::round(value)) << " |";
    std::cout << std::setw(column_width[3]) << value << " |";
    std::cout << '\n';
}

int main(const int argc, const char *const argv[])
{

    double refresh_rate{};
    if (argc > 2 && std::string_view(argv[1]) == "-c")
    {
        refresh_rate = std::stod(argv[2]);
        if (refresh_rate <= 0)
        {
            std::cerr << "Invalid refresh rate." << '\n';
            return 1;
        }
    }
    else
    {
        refresh_rate = query_display_refresh_rate();
        if (!refresh_rate)
        {
            std::cerr << "Couldn't find display's refresh rate." << '\n';
            return 1;
        }
    }

    const double rtss_vrr_fps{rtss_vrr_limit(refresh_rate)};

    const double sk_reflex_fps{sk_reflex_formula_limit(refresh_rate)};
    const double sk_vrr_v1_fps{sk_vrr_formula_v1_limit(sk_reflex_fps)};
    const double sk_vrr_v2_fps{sk_vrr_formula_v2_limit(sk_reflex_fps)};
    const double sk_vrr_v3_fps{sk_vrr_formula_v3_limit(sk_reflex_fps)};

    std::cout << std::fixed << std::setprecision(6);

    std::cout << "Display refresh rate: " << refresh_rate << " Hz\n";

    std::cout << std::right;

    print_row_separator();

    print_row_title();
    print_row_separator();

    print_row_data("RTSS VRR limit [5%]", rtss_vrr_fps);
    print_row_separator();

    print_row_data("SK Reflex limit", sk_reflex_fps);
    print_row_data("SK VRR limit [0.5%]", sk_vrr_v1_fps);
    print_row_data("SK VRR limit [1%]", sk_vrr_v2_fps);
    print_row_data("SK VRR limit [~0.5%]", sk_vrr_v3_fps);
    print_row_separator();

    return 0;
}
