#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <iomanip>

// Чтение значений из файла в вектор
std::vector<double> readValuesFromFile(const std::string& filename) {
    std::vector<double> values;
    std::ifstream file(filename);
    if (file.is_open()) {
        // Пропускаем первую строку с названием времени
        std::string line;
        std::getline(file, line);

        double value;
        while (file >> value) {
            values.push_back(value);
        }
        file.close();
    } else {
        std::cerr << "Could not open the file: " << filename << std::endl;
    }
    return values;
}

void writeValuesToFile(const std::string& filename, const std::vector<double>& values) {
    std::ofstream file(filename);
    if (file.is_open()) {
        // Извлекаем num_elements из имени файла
        std::size_t pos = filename.find("_num_elements_");
        if (pos == std::string::npos) {
            std::cerr << "Could not find num_elements in the filename: " << filename << std::endl;
            return;
        }
        pos += 14; // Длина "_num_elements_"

        std::string num_elements_str;
        while (filename[pos] != '_' && pos < filename.size()) {
            num_elements_str += filename[pos];
            ++pos;
        }

        int num_elements = std::stoi(num_elements_str);

        for (size_t i = 0; i < values.size(); ++i) {
            file << std::setw(10) << std::setprecision(8)  << values[i];
            // Добавляем переход на новую строку после каждого значения, кроме последнего в строке
            if ((i + 1) % num_elements == 0 && i != values.size() - 1) {
                file << std::endl;
            } else if (i != values.size() - 1) {
                file << " ";
            }
        }
        file.close();
    } else {
        std::cerr << "Could not open the file: " << filename << std::endl;
    }
}

// Основная функция сравнения файлов
void compareFiles(const std::string& explicitPath, const std::string& implicitPath, const std::string& outputPath) {
    std::vector<double> explicitValues = readValuesFromFile(explicitPath);
    std::vector<double> implicitValues = readValuesFromFile(implicitPath);

    if (explicitValues.empty()) {
        std::cerr << "Explicit file is empty or could not be read: " << explicitPath << std::endl;
        return;
    }
    if (implicitValues.empty()) {
        std::cerr << "Implicit file is empty or could not be read: " << implicitPath << std::endl;
        return;
    }

    if (explicitValues.size() != implicitValues.size()) {
        std::cerr << "Files have different number of values: " << explicitPath << " and " << implicitPath << std::endl;
        return;
    }

    std::vector<double> diffValues;
    for (size_t i = 0; i < explicitValues.size(); ++i) {
        diffValues.push_back(explicitValues[i] - implicitValues[i]);
    }

    writeValuesToFile(outputPath, diffValues);
}

int main() {
    const std::string timeStep = "0-00001";

    std::string baseExplicitPath = "../../Explicit_scheme/Results/time_step_" + timeStep + "_num_elements_50_50/time_step_" + timeStep + "_Time_";
    std::string baseImplicitPath = "../../Implicit_scheme/Results/time_step_" + timeStep + "_num_elements_50_50/time_step_" + timeStep + "_Time_";
    std::string outputBasePath = "time_step_" + timeStep + "/comp_explicit_vs_implicit_time_step_" + timeStep + "_Time_";
    std::string logFilePath = "time_step_" + timeStep + "/comparison_filenames.txt";

    // Проверка существования выходной директории
    std::filesystem::create_directories("time_step_" + timeStep);

    std::ofstream logFile(logFilePath);
    if (!logFile.is_open()) {
        std::cerr << "Could not open the log file: " << logFilePath << std::endl;
        return 1;
    }

    for (int time = 0; time <= 100; time += 10) {
        std::ostringstream timeStr;
        timeStr << time;

        std::string explicitPath = baseExplicitPath + timeStr.str() + "_num_elements_50.txt";
        std::string implicitPath = baseImplicitPath + timeStr.str() + "_num_elements_50.txt";
        std::string outputPath = outputBasePath + timeStr.str() + "_num_elements_50.txt";

        // Проверка существования входных файлов
        bool explicitFileExists = std::filesystem::exists(explicitPath);
        bool implicitFileExists = std::filesystem::exists(implicitPath);

        if (!explicitFileExists) {
            std::cerr << "Explicit file does not exist: " << explicitPath << std::endl;
            continue;
        }
        if (!implicitFileExists) {
            std::cerr << "Implicit file does not exist: " << implicitPath << std::endl;
            continue;
        }

        compareFiles(explicitPath, implicitPath, outputPath);

        // Запись пути выходного файла в лог
        logFile << outputPath << std::endl;
    }

    logFile.close();
    std::cout << "Comparison completed." << std::endl;

    std::cout << "Press any Enter to exit..." << std::endl;
    std::cin.get();
    return 0;
}
