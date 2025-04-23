#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

const int TOTAL_POSITIONS = 5000000;
const int NPOSITIONS = 10000;
using pair_t = std::array<double, 2>;
std::vector<int> coefficients = {};
std::vector<pair_t> parameters = {};

std::array<std::string, NPOSITIONS> fens = {};
std::array<double, NPOSITIONS> wdls = {};
//std::array<Trace, NPOSITIONS> posTraces = {};
double K = 2.25;


struct WdlMarker
{
    std::string marker;
    double wdl;
};

struct Coefficient{
	int16_t index;
	int16_t val;
};
struct Position {
	int coeff_begin;
	int coeff_end;
	double wdl;
	double phase, phase_value;
};

struct Dataset {
	std::vector<Coefficient> allCoefficients;
	std::vector<Position> positions;
};

Dataset dataset;

static const std::array<WdlMarker, 4> markers
{
    WdlMarker{"1.0", 1},

    WdlMarker{"1-0", 1},
    WdlMarker{"1/2-1/2", 0.5},
    WdlMarker{"0-1", 0}
};

static double get_fen_wdl(const std::string& original_fen){
    double wdl;
    bool marker_found = false;
    for (auto& marker : markers)
    {
        if (original_fen.find(marker.marker) != std::string::npos)
        {
            marker_found = true;
            wdl = marker.wdl;
        }
    }

    if(!marker_found)
    {
        std::stringstream ss(original_fen);
        while (!ss.eof())
        {
            std::string word;
            ss >> word;
            if (word.substr(0, 2) == "0.")
            {
                wdl = std::stod(word);
                marker_found = true;
            }
            else if (word.substr(0, 3) == "[0.")
            {
                wdl = std::stod(word.substr(1, word.size() - 2));
                marker_found = true;
            }
        }
    }
    return wdl;
}

std::string cleanup_fen(std::string& initial_fen){
    int space_count = 0;
    size_t pos = 0;
    for (size_t i = 0; i < initial_fen.size(); ++i) {
        if (initial_fen[i] == ' ') {
            ++space_count;
        }
        if (space_count == 4) {
            pos = i;
            break;
        }
    }
    const auto clean_fen = initial_fen.substr(0, pos);
    return clean_fen;
}

void read_data(int start){
	std::ifstream file("../data/quiet-labeled.epd");
	std::string line;
	int i = 0;
	if (file.is_open()) {
		while (std::getline(file, line) && i-start < NPOSITIONS) {
			if (i > start-1){
				fens[i-start] = cleanup_fen(line);
				wdls[i-start] = get_fen_wdl(line);

				// if (i % 10000 == 0)
				// 	std::cout << "Loaded " << i << std::endl;
			}
			i++;
		}
	file.close();
	} else {
	std::cerr << "Unable to open file" << std::endl;
	}
}
void get_single(int s, int &index){
	coefficients[index] = s;
	index++;
}
template<std::size_t SIZE>
void get_array(std::array<std::array<int, 2>, SIZE> &arr, int &index){
	for (int i=0;i<arr.size();i++)
		get_single(arr[i][0] - arr[i][1], index);
}

void get_eval_trace(){
	int index = 0;
	std::fill(coefficients.begin(), coefficients.end(), 0);
	get_array(EvalTrace.pieceTypeValues, index);
	get_array(EvalTrace.kingLineDanger, index);
	get_array(EvalTrace.PSQT[0], index);
	get_array(EvalTrace.PSQT[1], index);
	get_array(EvalTrace.PSQT[2], index);
	get_array(EvalTrace.PSQT[3], index);
	get_array(EvalTrace.PSQT[4], index);
	get_array(EvalTrace.PSQT[5], index);

	get_array(EvalTrace.mobilityBonus[0], index);
	get_array(EvalTrace.mobilityBonus[1], index);
	get_array(EvalTrace.mobilityBonus[2], index);
	get_array(EvalTrace.mobilityBonus[3], index);

	get_array(EvalTrace.passers, index);

	get_single(EvalTrace.bishopPair[0]-EvalTrace.bishopPair[1], index);

	get_array(EvalTrace.pawnPhalanx, index);
	get_array(EvalTrace.kingAttacks, index);
	
}


double linearEvaluation(Trace trace){
	double mg = 0;
	double eg = 0;
	for (int i=0;i<coefficients.size();i++){
		mg += coefficients[i] * parameters[i][0];
		eg += coefficients[i] * parameters[i][1];
	}
	double eval = (mg * trace.phase + eg*(256.0f-trace.phase))/256.0f;
	return eval;
}


void load_data(){
	Board board;
	std::ifstream file("../data/E1MResolved.book");
	std::string line;
	int i = 0;
	if (file.is_open()){
		while (std::getline(file, line) && i < TOTAL_POSITIONS){
			std::string Fen = cleanup_fen(line);
			double wdl = get_fen_wdl(line);
			const char *fen = Fen.c_str();
			board.parseFen(fen);
			evaluate(board, board.sideToMove);
			get_eval_trace();
			Position pos;
			pos.coeff_begin = dataset.allCoefficients.size();
			for (int q=0;q<coefficients.size();q++){
				if (coefficients[q] != 0){
					Coefficient c;
					c.index = q;
					c.val = coefficients[q];
					dataset.allCoefficients.push_back(c);
				}
			}
			pos.coeff_end = dataset.allCoefficients.size();
			pos.wdl = wdl;
			pos.phase_value = EvalTrace.phase_value;
			pos.phase = EvalTrace.phase;
			dataset.positions.push_back(pos);
			if (i % 100000 == 0){
				printf("Loaded %d\n", i);
			}
			i++;
		}
	}
	file.close();
	// for (int i=0;i<NPOSITIONS;i++){
	// 	const char *fen = fens[i].c_str();
	// 	board.parseFen(fen);
	// 	evaluate(board, board.sideToMove);
	// 	get_eval_trace();
	// 	Position pos;
	// 	pos.coeff_begin = dataset.allCoefficients.size();
	// 	for (int q=0;q<coefficients.size();q++){
	// 		if (coefficients[q] != 0){
	// 			Coefficient c;
	// 			c.index = q;
	// 			c.val = coefficients[q];
	// 			dataset.allCoefficients.push_back(c);
	// 		}
	// 	}
	// 	pos.coeff_end = dataset.allCoefficients.size();
	// 	pos.wdl = wdls[i];
	// 	pos.phase_value = EvalTrace.phase_value;
	// 	pos.phase = EvalTrace.phase;
	// 	dataset.positions.push_back(pos);
	// }
	// printf("Loaded %d\n", (j+1)*NPOSITIONS);

}

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

double sigmoid(double K_, double eval){
	return 1 / (1 + exp(-K_*eval / 400.0));
}


double linearEvaluationData(int index){
	double mg = 0;
	double eg = 0;
	Position pos = dataset.positions[index];
	for (int i=pos.coeff_begin;i<pos.coeff_end;i++){
		Coefficient c = dataset.allCoefficients[i];
		mg += c.val * parameters[c.index][0];
		eg += c.val * parameters[c.index][1];
	}
	return (mg * pos.phase + eg*(256.0f-pos.phase))/256.0f;
}

double linearEvaluationError(){
	double total = 0;
	for (int i=0;i<TOTAL_POSITIONS;i++){
		double eval = linearEvaluationData(i);
		total += pow(dataset.positions[i].wdl - sigmoid(K, eval), 2);
	}
	return total / (double)TOTAL_POSITIONS;
}


double computeOptimalK(){
	const double rate = 100, delta = 1e-5, deviation_goal = 1e-6;
	double K_ = 2.5, deviation = 1;

	while (fabs(deviation) > deviation_goal){
		K = K_ + delta; // lol im such a bad programmer;
		double up = linearEvaluationError();
		K = K_ - delta;
		double down = linearEvaluationError();
		deviation = (up-down)/(2*delta);
		K_ -= deviation * rate;
		printf("Optimal K = %f\n", K_);
	}
	K = K_;
	return K_;
}
// double linearEvaluationError(){
// 	double total = 0;
// 	Board board;

// 	for (int j=0;j<TOTAL_POSITIONS/NPOSITIONS;j++){
// 		read_data(j*NPOSITIONS);
// 		for (int i=0;i<NPOSITIONS;i++){
// 			const char *fen = fens[i].c_str();
// 			board.parseFen(fen);
// 			evaluate(board, board.sideToMove);
// 			get_eval_trace();
// 			double eval = linearEvaluation(EvalTrace);
// 			total += pow(wdls[i] - sigmoid(K, eval), 2);
// 		}
		
// 	}

// 	//printf("Total Error %f\n", total);
// 	return total / ((double)TOTAL_POSITIONS);
// }

// void updateSingleGradient(Trace trace, double res, double gradient[][2]){
// 	double E = linearEvaluation(trace);
// 	double S = sigmoid(K, E);
// 	double X = (res - S) * S * (1-S);

// 	double mg_base = X * trace.phase_value / 24.0;
// 	double eg_base = X * (1-trace.phase_value / 24.0);
// 	for (int i=0;i<coefficients.size();i++){
// 		gradient[i][0] += mg_base * coefficients[i];
// 		gradient[i][1] += eg_base * coefficients[i];
// 	}
// }
void updateSingleGradient(int index, double gradient[][2]){
	Position pos = dataset.positions[index];
	double E = linearEvaluationData(index);
	double S = sigmoid(K, E);
	double X = (pos.wdl - S) * S * (1-S);

	double mg_base = X * pos.phase_value / 24.0;
	double eg_base = X * (1-pos.phase_value / 24.0);
	for (int i=pos.coeff_begin;i<pos.coeff_end;i++){
		Coefficient c = dataset.allCoefficients[i];
		gradient[c.index][0] += mg_base * c.val;
		gradient[c.index][1] += eg_base * c.val;
	}
}
double computeGradient(double gradient[][2]){
	clock_t start = Now();
	for (int i=0;i<TOTAL_POSITIONS;i++){
		updateSingleGradient(i, gradient);
	}
	return (double)TOTAL_POSITIONS/(double)TimeSince(start)*100.0;
}
// void computeGradient(double gradient[][2]){
// 	Board board;
// 	const int total = coefficients.size();
// 	clock_t start = Now();
// 	for (int j=0;j<TOTAL_POSITIONS/NPOSITIONS;j++){
// 		read_data(j*NPOSITIONS);
// 		for (int i=0;i<NPOSITIONS;i++){
// 			const char *fen = fens[i].c_str();
// 			board.parseFen(fen);
// 			evaluate(board, board.sideToMove); // Get all traces
// 			get_eval_trace();
// 			updateSingleGradient(EvalTrace, wdls[i], gradient);
// 		}
// 		//printf("Computed %d ", j);
// 	}
// 	printf("\n");
// 	printf("Finished computing gradients. %f positions per second\n", (double)TOTAL_POSITIONS/(double)TimeSince(start)*100.0);
// 	// for (int i=0;i<total;i++)
// 	// 	printf("%f %f\n", gradient[i][0], gradient[i][1]);
// }


//-----print stuff

void printArray(int s, int e){
	printf("{");
	for (int i=s;i<e;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("};\n");
}
void printPST(){
	printf("PSQT = {\n{");
	for (int i=34;i<34+64;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("},\n");
	printf("{");
	for (int i=34+64;i<34+64*2;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("},\n");
	printf("{");
	for (int i=34+64*2;i<34+64*3;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("},\n");
	printf("{");
	for (int i=34+64*3;i<34+64*4;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("},\n");
	printf("{");
	for (int i=34+64*4;i<34+64*5;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("},\n");
	printf("{");
	for (int i=34+64*5;i<34+64*6;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("}\n};\n");
}
void printMobility(){
	printf("mobilityBonus = {\n{");
	for (int i=418;i<418+32;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("},\n");
	printf("{");
	for (int i=418+32;i<418+32*2;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("},\n");
	printf("{");
	for (int i=418+32*2;i<418+32*3;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("},\n");
	printf("{");
	for (int i=418+32*3;i<418+32*4;i++){
		printf("S(%d, %d), ", (int)parameters[i][0], (int)parameters[i][1]);
	}
	printf("}\n};\n");
}
void printParameters(){
	printf("pieceTypeValues = ");
	printArray(0, 6);
	printf("kingLineDanger = ");
	printArray(6, 34);
	printPST();
	printMobility();
	printf("passers = ");
	printArray(546, 550);
	printf("bishopPair = S(%d, %d)\n", (int)parameters[550][0], (int)parameters[550][1]);
	printf("pawnPhalanx = ");
	printArray(551, 558);
	printf("kingAttacks = ");
	printArray(558, 562);

}
void tune(){
	const int total = coefficients.size();
	double LR = 0.1;
	double BETA_1 = 0.9;
	double BETA_2 = 0.999;

	double error, rate = LR;
	double momentum[total][2] = {0};
	double velocity[total][2] = {0};
	//printParameters();
	for (int epoch=0;epoch<=10000;epoch++){
		double gradient[total][2] = {0};
		
		//printf("Computing Gradients\n");
		double pps = computeGradient(gradient);
		for (int i=0;i<coefficients.size();i++){
			double mg_grad = (-K / 200.0) * gradient[i][0] / (double)TOTAL_POSITIONS;
			double eg_grad = (-K / 200.0) * gradient[i][1] / (double)TOTAL_POSITIONS;

			momentum[i][0] = BETA_1 * momentum[i][0] + (1.0 - BETA_1) * mg_grad;
			momentum[i][1] = BETA_1 * momentum[i][1] + (1.0 - BETA_1) * eg_grad;

			velocity[i][0] = BETA_2 * velocity[i][0] + (1.0 - BETA_2) * mg_grad * mg_grad;
			velocity[i][1] = BETA_2 * velocity[i][1] + (1.0 - BETA_2) * eg_grad * eg_grad;

			parameters[i][0] -= rate * momentum[i][0] / (1e-8 + sqrt(velocity[i][0]));
			parameters[i][1] -= rate * momentum[i][1] / (1e-8 + sqrt(velocity[i][1]));
		}

		if (epoch % 100 == 0){
			printParameters();
		}
		if (epoch % 50){
			error = linearEvaluationError();
			printf("Epoch %d\n%f positions per second\nError %.10f\n", epoch, pps, error);
		}
		
	}
}
//--


void intialize_tuner(){
	srand(1);
	coefficients.clear();
	parameters.clear();

	pair_t rng = {0,0};
	int num = 0;
	// Piece Values
	rng[0] = 0;
	rng[1] = 0;
	num = 6;
	coefficients.insert(coefficients.end(), num, 0); 
	parameters.insert(parameters.end(), num, rng); 
	for (int i=0;i<num;i++){
		parameters[i][0] = MgScore(pieceTypeValues[i]);
		parameters[i][1] = EgScore(pieceTypeValues[i]);
	}

	// kingLineDanger
	num = 28;
	coefficients.insert(coefficients.end(), num, 0); 
	parameters.insert(parameters.end(), num, rng);
	for (int i=6;i<6+num;i++){
		parameters[i][0] = MgScore(kingLineDanger[i-6]);
		parameters[i][1] = EgScore(kingLineDanger[i-6]);
	}

	// PSQT
	num = 64 * 6;
	coefficients.insert(coefficients.end(), num, 0); 
	parameters.insert(parameters.end(), num, rng);
	for (int j=0;j<6;j++){
		for (int i=0;i<64;i++){
			parameters[34+i+64*j][0] = MgScore(PSQT[j][i]);
			parameters[34+i+64*j][1] = EgScore(PSQT[j][i]);
		}
	}

	// Mobility Bonus
	num = 32 * 4;
	coefficients.insert(coefficients.end(), num, 0); 
	parameters.insert(parameters.end(), num, rng);
	for (int j=0;j<4;j++){
		for (int i=0;i<32;i++){
			parameters[418+i+32*j][0] = MgScore(mobilityBonus[j][i]);
			parameters[418+i+32*j][1] = EgScore(mobilityBonus[j][i]);
		}
	}
	// Passer Pawns Rank
	num = 4;
	coefficients.insert(coefficients.end(), num, 0); 
	parameters.insert(parameters.end(), num, rng);
	for (int i=546;i<546+num;i++){
		parameters[i][0] = MgScore(passers[i-546]);
		parameters[i][1] = EgScore(passers[i-546]);
	}
	// bishop pair
	num = 1;
	coefficients.insert(coefficients.end(), num, 0); 
	parameters.insert(parameters.end(), num, rng); 
	parameters[550][0] = 50;
	parameters[550][1] = 66;

	num = 7;
	coefficients.insert(coefficients.end(), num, 0); 
	parameters.insert(parameters.end(), num, rng);
	for (int i=551;i<551+num;i++){
		parameters[i][0] = MgScore(pawnPhalanx[i-551]);
		parameters[i][1] = EgScore(pawnPhalanx[i-551]);
	}

	num = 4;
	coefficients.insert(coefficients.end(), num, 0); 
	parameters.insert(parameters.end(), num, rng);
	for (int i=558;i<558+num;i++){
		parameters[i][0] = MgScore(kingAttacks[i-558]);
		parameters[i][1] = EgScore(kingAttacks[i-558]);
	}


	printParameters();
}

