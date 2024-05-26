#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>
#include <omp.h>      // Для OpenMP
#include <algorithm>  // Для std::min
#include <chrono>     // Для измерения времени выполнения
#include <filesystem> // Для работы с файловой системой

namespace fs = std::filesystem;

// Объявление переменных
int L, H, num_steps, num_dx, num_dy, width = 10, precision = 8;
double alpha, T_in, T_init, conv_coeff, t_max, dx, dy, dt, courant_number;
std::ofstream namesFile;     // Глобальная переменная для файла с названиями файлов
std::string outputDirectory; // Папка для сохранения файлов

void readParameters()
{
    std::ifstream input("input_im.txt");
    std::string line;
    if (input.is_open())
    {
        while (getline(input, line))
        {
            // Игнорирование комментариев и пустых строк
            if (line[0] == '#' || line.empty())
                continue;

            std::istringstream iss(line);
            if (!(iss >> L >> H >> alpha >> T_in >> T_init >> t_max >> dt >> num_dx >> num_dy))
            {
                // Обработка ошибки, если не удалось прочитать значения
                std::cerr << "Error reading parameters from file." << std::endl;
                exit(1); // Выход с ошибкой
            }
            break; // Выходим из цикла после чтения первой строки с данными
        }
        input.close();

        // Вычисление dx, dy и dt на основании считанных значений
        dx = static_cast<double>(L) / (num_dx - 1);
        dy = static_cast<double>(H) / (num_dy - 1);
        num_steps = t_max / dt + 1;

        // Вычисление числа Куранта
        // courant_number = alpha * dt / (dx * dx);
        // std::cerr << "courant number:    " << courant_number << std::endl;
    }
    else
    {
        std::cerr << "Unable to open input.txt" << std::endl;
        exit(1); // Выход с ошибкой
    }
}

void initialize(std::vector<std::vector<double>> &T)
{
    for (int i = 0; i < num_dx; ++i)
    {
        T[i][0] = T_in;
        for (int j = 0; j < num_dy; ++j)
        {
            T[0][j] = T_in; // Граничное условие нагревающей стороны
        }
    }
}

void update(std::vector<std::vector<double>> &T, int step, bool isLastTimeStep)
{
    int N = num_dx;
    int M = num_dy;
    double alpha_dt_dx2 = alpha * dt / (dx * dx);
    double alpha_dt_dy2 = alpha * dt / (dy * dy);
    std::vector<std::vector<double>> T_new(N, std::vector<double>(M, 0.0));
    std::vector<std::vector<double>> T_temp(N, std::vector<double>(M, 0.0));

// Первый этап: обновление вдоль оси x
#pragma omp parallel for
    for (int j = 0; j < M; ++j)
    {
        std::vector<double> a(N, -alpha_dt_dx2);
        std::vector<double> b(N, 1 + 2 * alpha_dt_dx2);
        std::vector<double> c(N, -alpha_dt_dx2);
        std::vector<double> d(N);

        // Заполнение правой части
        for (int i = 1; i < N - 1; ++i)
        {
            d[i] = T[i][j];
        }

        // Граничные условия
        b[0] = b[N - 1] = 1.0;
        c[0] = a[N - 1] = 0.0;
        d[0] = T[0][j];
        d[N - 1] = T[N - 1][j];

        // Прямая прогонка
        for (int i = 1; i < N; ++i)
        {
            double m = a[i] / b[i - 1];
            b[i] -= m * c[i - 1];
            d[i] -= m * d[i - 1];
        }

        // Обратная прогонка
        T_temp[N - 1][j] = d[N - 1] / b[N - 1];
        for (int i = N - 2; i >= 0; --i)
        {
            T_temp[i][j] = (d[i] - c[i] * T_temp[i + 1][j]) / b[i];
        }
    }

// Второй этап: обновление вдоль оси y
#pragma omp parallel for
    for (int i = 0; i < N; ++i)
    {
        std::vector<double> a(M, -alpha_dt_dy2);
        std::vector<double> b(M, 1 + 2 * alpha_dt_dy2);
        std::vector<double> c(M, -alpha_dt_dy2);
        std::vector<double> d(M);

        // Заполнение правой части
        for (int j = 1; j < M - 1; ++j)
        {
            d[j] = T_temp[i][j];
        }

        // Граничные условия
        b[0] = b[M - 1] = 1.0;
        c[0] = a[M - 1] = 0.0;
        d[0] = T_temp[i][0];
        d[M - 1] = T_temp[i][M - 1];

        // Прямая прогонка
        for (int j = 1; j < M; ++j)
        {
            double m = a[j] / b[j - 1];
            b[j] -= m * c[j - 1];
            d[j] -= m * d[j - 1];
        }

        // Обратная прогонка
        T_new[i][M - 1] = d[M - 1] / b[M - 1];
        for (int j = M - 2; j >= 0; --j)
        {
            T_new[i][j] = (d[j] - c[j] * T_new[i][j + 1]) / b[j];
        }
    }

    // Копирование обновленных значений обратно в основную матрицу
    T = T_new;
}

void print(const std::vector<std::vector<double>> &T, int step)
{
    std::cout << "Time: " << dt * step << "   | step:    " << step << ":" << std::endl;
    // for (int j = 0; j < num_dy - 1; ++j) {
    //     for (int i = 0; i < num_dx - 1; ++i) {
    //         std::cout<< std::setw(width) << std::setprecision(precision) << T[i][j] << "\t";
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << std::endl;
}

// Функция для замены точек на дефисы
std::string replaceDotsWithDashes(const std::string &input)
{
    std::string output = input;
    std::replace(output.begin(), output.end(), '.', '-');
    return output;
}

// Функция для форматирования чисел, убирающая незначащие нули
std::string formatNumber(double number)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << number;
    std::string str = oss.str();
    str.erase(str.find_last_not_of('0') + 1, std::string::npos); // Удаление завершающих нулей
    if (str.back() == '.')
    {
        str.pop_back(); // Удаление точки, если она осталась в конце
    }
    return str;
}

// Генерация имени папки
std::string generateOutputDirectory(double dt, int num_dx, int num_dy)
{
    std::ostringstream dirname;
    dirname << "Results/time_step_" << replaceDotsWithDashes(formatNumber(dt))
            << "_num_elements_" << num_dx << "_" << num_dy;
    return dirname.str();
}

// Запись диагональных элементов в отдельный файл
void writeDiagonalToFile(const std::vector<std::vector<double>> &T, int step)
{
    std::ostringstream filename;
    filename << outputDirectory << "/Diagonal_Elements_t-step_" << replaceDotsWithDashes(formatNumber(dt))
             << "_num_elements_" << num_dx - 1 << ".txt";
    std::ofstream outFile(filename.str(), std::ios_base::app);

    if (!outFile.is_open())
    {
        std::cerr << "Unable to open output file: " << filename.str() << std::endl;
        exit(1);
    }

    for (int i = 0; i < num_dx && i < num_dy; i += 1)
    {
        outFile << std::setw(width) << std::setprecision(precision) << T[i][i] << "\t";
    }
    outFile << std::endl;
    outFile.close();
}

void printToFile(const std::vector<std::vector<double>> &T, int step)
{
    std::ostringstream filename;
    filename << outputDirectory << "/time_step_" << replaceDotsWithDashes(formatNumber(dt))
             << "_Time_" << replaceDotsWithDashes(formatNumber(dt * step))
             << "_num_elements_" << num_dx - 1 << ".txt";
    std::ofstream outFile(filename.str());

    if (!outFile.is_open())
    {
        std::cerr << "Unable to open output file: " << filename.str() << std::endl;
        exit(1);
    }

    outFile << "Time step " << dt * step << ":" << std::endl;
    for (int j = 0; j < num_dy - 1; ++j)
    {
        for (int i = 0; i < num_dx - 1; ++i)
        {
            outFile << std::setw(width) << std::setprecision(precision) << T[i][j] << "\t";
        }
        outFile << std::endl;
    }
    outFile.close();

    // Запись имени файла в namesFile
    namesFile << "\"" << filename.str() << "\"" << std::endl;
}

int main()
{
    // Чтение параметров
    readParameters();
    double dt_comparison = dt;
    // Создание основной папки "Results"
    fs::create_directories("Results");
    while (dt <= 1)
    {
        // Создание выходной папки для текущих параметров
        outputDirectory = generateOutputDirectory(dt, num_dx, num_dy);
        fs::create_directories(outputDirectory); // Создание папки и всех необходимых родительских директорий

        // Создание имени файла для записи имен файлов
        std::ostringstream namesFilename;
        namesFilename << outputDirectory << "/names_of_time_step_files_" << replaceDotsWithDashes(formatNumber(dt)) << ".txt";
        namesFile.open(namesFilename.str()); // Открытие файла для записи имен файлов
        if (!namesFile.is_open())
        {
            std::cerr << "Unable to open " << namesFilename.str() << std::endl;
            return 1; // Выход с ошибкой
        }

        // Вывод числа Куранта
        std::cout << "Courant Number: " << courant_number << std::endl;
        num_dx += 1;
        num_dy += 1;

        // Начало измерения времени
        auto start = std::chrono::high_resolution_clock::now();

        // Инициализация матрицы температур
        std::vector<std::vector<double>> T(num_dx, std::vector<double>(num_dy, T_init));

        // Установка начальных и граничных условий
        initialize(T);

        print(T, 0);
        printToFile(T, 0);
        writeDiagonalToFile(T, 0);

        // Расчет температурного поля по времени
        for (int step = 0; step <= num_steps; ++step)
        {
            bool isLastTimeStep = (step == num_steps);
            update(T, step, isLastTimeStep);

            if ((step % (num_steps / 100) == 0) && (step != 0))
            {
                print(T, step);
                printToFile(T, step);
                writeDiagonalToFile(T, step);
            }
        }
        num_dx -= 1;
        num_dy -= 1;
        // Запись параметров в файл для Python-скрипта
        std::ofstream paramsFile(/*outputDirectory + */ "params.txt");
        if (paramsFile.is_open())
        {
            paramsFile << dt << "\n"
                       << num_dx << "\n"
                       << t_max << "\n";
            paramsFile.close();
        }
        else
        {
            std::cerr << "Unable to open params file" << std::endl;
            return 1;
        }
        std::string command = "python ./Images_and_gif_of_schemes.py";
        int result = system(command.c_str());

        if (result != 0)
        {
            std::cerr << "Error running Python script: " << result << std::endl;
            return 1; // Выход с ошибкой
        }

        // Уменьшение шага времени
        dt *= 10;
        num_steps = t_max / dt + 1;
        outputDirectory = generateOutputDirectory(dt, num_dx, num_dy);
        fs::create_directories(outputDirectory);

        // Создание имени файла для записи имен файлов
        namesFilename.str("");
        namesFilename.clear();
        namesFilename << outputDirectory << "/names_of_time_step_files_" << replaceDotsWithDashes(formatNumber(dt)) << ".txt";
        namesFile.open(namesFilename.str());
        if (!namesFile.is_open())
        {
            std::cerr << "Unable to open " << namesFilename.str() << std::endl;
            return 1;
        }

        // Конец измерения времени
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;

        // Закрытие файла с именами файлов
        namesFile.close();

        // Создание имени файла для записи времени выполнения
        std::ostringstream timeFilename;
        timeFilename << outputDirectory << "/Time_of_calcul_" << replaceDotsWithDashes(formatNumber(t_max))
                     << "_" << replaceDotsWithDashes(formatNumber(dt)) << "_" << num_dy - 1 << ".txt";
        std::ofstream timeFile(timeFilename.str());

        if (!timeFile.is_open())
        {
            std::cerr << "Unable to open time output file: " << timeFilename.str() << std::endl;
            return 1; // Выход с ошибкой
        }

        // Запись времени выполнения в файл
        timeFile << "Elapsed time: " << elapsed_seconds.count() << " seconds" << std::endl;
        timeFile.close();
    }
    return 0;
}
