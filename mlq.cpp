#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <sstream>

using namespace std;

// Estructura de proceso
struct Process {
    string id;
    int burst_time;
    int arrival_time;
    int queue_level;
    int priority;
    int wait_time;
    int completion_time;
    int response_time;
    int turnaround_time;
    int remaining_time;
};

// Función para leer procesos desde un archivo
vector<Process> readProcessesFromFile(const string& filename) {
    vector<Process> processes;
    ifstream file(filename);
    string line;
    
    while (getline(file, line)) {
        if (line[0] == '#') continue; // Saltar comentarios
        
        stringstream ss(line);
        string id;
        int burst_time, arrival_time, queue_level, priority;
        char delimiter;
        
        getline(ss, id, ';');
        ss >> burst_time >> delimiter >> arrival_time >> delimiter >> queue_level >> delimiter >> priority;
        
        Process process = {id, burst_time, arrival_time, queue_level, priority, 0, 0, -1, 0, burst_time};
        processes.push_back(process);
    }
    
    return processes;
}

// Round Robin Scheduler con Quantum
void round_robin(vector<Process>& processes, int quantum, int& current_time) {
    queue<Process*> rr_queue;
    for (auto& process : processes) {
        if (process.arrival_time <= current_time && process.remaining_time > 0) {
            rr_queue.push(&process);
        }
    }
    
    while (!rr_queue.empty()) {
        Process* process = rr_queue.front();
        rr_queue.pop();
	
	if (process->response_time == -1) {
            process->response_time = current_time - process->arrival_time;
        }
        
        if (process->remaining_time <= quantum) {
            current_time += process->remaining_time;
            process->completion_time = current_time;
            process->remaining_time = 0;
        } else {
            current_time += quantum;
            process->remaining_time -= quantum;
            // Volver a encolar el proceso si no ha terminado
            rr_queue.push(process);
        }
        
        // Actualizar métricas
        process->turnaround_time = process->completion_time - process->arrival_time;
        process->wait_time = process->turnaround_time - process->burst_time;
    }
}

// First Come First Served (FCFS) Scheduler
void fcfs(vector<Process>& processes, int& current_time) {
    for (auto& process : processes) {
        if (process.arrival_time <= current_time && process.remaining_time > 0) {
	    if (process.response_time == -1) {
                process.response_time = current_time - process.arrival_time;
            }

            current_time += process.remaining_time;
            process.completion_time = current_time;
            process.remaining_time = 0;
            
            // Actualizar métricas
            process.turnaround_time = process.completion_time - process.arrival_time;
            process.wait_time = process.turnaround_time - process.burst_time;
        }
    }
}

// Función para dividir los procesos en sus respectivas colas
void processQueues(vector<Process>& processes) {
    vector<Process> queue1, queue2, queue3;
    
    // Clasificar los procesos por cola
    for (auto& process : processes) {
        if (process.queue_level == 1)
            queue1.push_back(process);
        else if (process.queue_level == 2)
            queue2.push_back(process);
        else if (process.queue_level == 3)
            queue3.push_back(process);
    }

    int current_time = 0;

    // Procesar Cola 1: RR(3)
    round_robin(queue1, 3, current_time);

    // Procesar Cola 2: RR(5)
    round_robin(queue2, 5, current_time);

    // Procesar Cola 3: FCFS
    fcfs(queue3, current_time);

    // Combinar los resultados
    processes.clear();
    processes.insert(processes.end(), queue1.begin(), queue1.end());
    processes.insert(processes.end(), queue2.begin(), queue2.end());
    processes.insert(processes.end(), queue3.begin(), queue3.end());
}

// Función para imprimir resultados a un archivo
void writeResultsToFile(const vector<Process>& processes, ofstream& file) {

    file << "# etiqueta; BT; AT; Q; Pr; WT; CT; RT; TAT\n";
    
    double total_wt = 0, total_ct = 0, total_rt = 0, total_tat = 0;
    
    for (const auto& process : processes) {
        file << process.id << ";" << process.burst_time << ";" << process.arrival_time << ";" 
             << process.queue_level << ";" << process.priority << ";" << process.wait_time << ";" 
             << process.completion_time << ";" << process.response_time << ";" << process.turnaround_time << "\n";
        
        total_wt += process.wait_time;
        total_ct += process.completion_time;
        total_rt += process.response_time;
        total_tat += process.turnaround_time;
    }
    
    // Imprimir promedios
    file << "WT=" << total_wt / processes.size() << "; "
         << "CT=" << total_ct / processes.size() << "; "
         << "RT=" << total_rt / processes.size() << "; "
         << "TAT=" << total_tat / processes.size() << ";\n";
}

// Función principal
int main() {
    vector<string> input_files = {"mlq001.txt", "mlq002.txt", "mlq003.txt"};
    string output_filename = "mlq_results.txt";
    
    // Abrir el archivo de resultados en modo de adición
    ofstream resultFile(output_filename, ios::app);
    if (!resultFile.is_open()) {
        cerr << "Error al abrir el archivo de resultados." << endl;
        return 1;
    }

    // Iterar sobre cada archivo de entrada
    for (const string& input_filename : input_files) {
        cout << "Procesando archivo: " << input_filename << endl;

        // Leer los procesos desde el archivo de entrada
        vector<Process> processes = readProcessesFromFile(input_filename);

        // Procesar las colas de procesos
        processQueues(processes);

        // Escribir resultados en el archivo de salida usando la función existente
        resultFile << "# Archivo: " << input_filename << endl;
        writeResultsToFile(processes, resultFile); // Cambiar a la función que toma el archivo de salida directamente

        // Para mantener la separación en el archivo de resultados
        resultFile << endl; // Línea en blanco para separación entre diferentes archivos
    }

    resultFile.close();
    cout << "Planificación completada. Resultados guardados en " << output_filename << endl;
    
    return 0;
}
