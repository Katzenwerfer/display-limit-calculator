#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>

#include <dxgi.h>

double get_dxgi_refresh_rate()
{
    IDXGIFactory *pFactory = nullptr;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory)))
    {
        return 0.0;
    }

    IDXGIAdapter *pAdapter = nullptr;
    if (FAILED(pFactory->EnumAdapters(0u, &pAdapter)))
    {
        pFactory->Release();
        return 0.0;
    }

    IDXGIOutput *pOutput = nullptr;
    if (FAILED(pAdapter->EnumOutputs(0u, &pOutput)))
    {
        pAdapter->Release();
        pFactory->Release();
        return 0.0;
    }

    DXGI_OUTPUT_DESC outputDesc;
    pOutput->GetDesc(&outputDesc);

    DXGI_MODE_DESC targetMode;
    targetMode.Width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    targetMode.Height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;
    targetMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    targetMode.RefreshRate.Numerator = 0u;
    targetMode.RefreshRate.Denominator = 0u;
    targetMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    targetMode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    DXGI_MODE_DESC closestMatch;
    pOutput->FindClosestMatchingMode(&targetMode, &closestMatch, nullptr);

    pOutput->Release();
    pAdapter->Release();
    pFactory->Release();

    const double numerator = static_cast<double>(closestMatch.RefreshRate.Numerator);
    const double denominator = static_cast<double>(closestMatch.RefreshRate.Denominator);
    return numerator / denominator;
}

double get_gdi_refresh_rate()
{
    DEVMODE devMode;
    ZeroMemory(&devMode, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode))
    {
        return static_cast<double>(devMode.dmDisplayFrequency);
    }
    return 0.0;
}

double query_display_refresh_rate()
{
    double refresh_rate = get_dxgi_refresh_rate();
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

constexpr double five_percent_limit(const double &refresh_rate)
{
    return refresh_rate * 0.95;
}

constexpr double reflex_formula_limit(const double &refresh_rate)
{
    return refresh_rate - (refresh_rate * refresh_rate) / 3600.0;
}

constexpr double vrr_formula_limit(const double &reflex_fps)
{
    return reflex_fps * 0.995;
}

void print_row(const std::string &label, const double &value, const std::array<int, 4> &width)
{
    std::cout << std::setw(width[0]) << (label + " |");
    std::cout << std::setw(width[1] - 2) << static_cast<int>(value) << " |";
    std::cout << std::setw(width[2] - 2) << static_cast<int>(std::round(value)) << " |";
    std::cout << std::setw(width[3] - 2) << value << " |";
    std::cout << '\n';
}

int main()
{
    const double refresh_rate = query_display_refresh_rate();
    if (!refresh_rate)
    {
        std::cerr << "Couldn't find display's refresh rate." << '\n';
        return 1;
    }

    const double percent_fps = five_percent_limit(refresh_rate);
    const double reflex_fps = reflex_formula_limit(refresh_rate);
    const double optimal_fps = vrr_formula_limit(reflex_fps);

    std::cout << std::fixed << std::setprecision(3);

    std::cout << "Detected display refresh rate: " << refresh_rate << " Hz\n";

    std::cout << std::right;

    const std::array<int, 4> width = {22, 16, 14, 21};

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
