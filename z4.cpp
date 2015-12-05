#include "stdafx.h"
#include "time.h"

#include <omp.h>
#include <iostream>
#include <ctime>
#include <string>


using namespace std;

inline bool isInIntArray(int* a, int aSize, int value)
{
	for (int i = 0; i < aSize; i++)
	{
		if (a[i] == value)
			return true;
	}

	return false;
}

struct town
{
	int id = 0, x = 0, y = 0;

	town() {}
	town(int id, int x, int y) : id(id), x(x), y(y) {}

	void setData(int newId, int newX, int newY)
	{
		id = newId;
		x = newX;
		y = newY;
	}

	float countEuclideanDistance(town t)
	{
		return (float)sqrt((x - t.x)*(x - t.x) + (y - t.y)*(y - t.y));
	}

	// Only for debugging issues
	string toString()
	{
		return "Town's data: id = " + to_string(id) + ", x = " + to_string(x) + ", y = " + to_string(y) + "\n";
	}
};

struct solution
{
	town* towns;
	unsigned int townsNo;
	int* townsOrder;
	float evaluationValue;
	
	solution() {}
	
	solution(int townsNumber, town* towns) : townsNo(townsNumber), towns(towns)
	{
		townsOrder = new int[townsNo];
		initiateTownsOrder();
		evaluate();
	}

	inline void initiateTownsOrder()
	{
		populate();
		shuffle();
	}

	inline void populate()
	{
		for (int i = 0; i < townsNo; i++)
			townsOrder[i] = i;
	}

	inline void shuffle()
	{
		for (int i = townsNo - 1; i > 0; i--)
		{
			int j = rand() % i;
			swap(townsOrder[i], townsOrder[j]);
		}
	}

	void evaluate()
	{
		float sum = 0.0;
		int i = 0;
	
		for(; i < townsNo - 2; i++)
			sum += towns[townsOrder[i]].countEuclideanDistance(towns[townsOrder[i + 1]]);

		evaluationValue = sum;
	}

	void pmxCross(solution* parent)
	{
		//Get a substring of towns borders r1 and r2
		int r1 = rand() % townsNo, r2 = rand() % townsNo;
		int i = 0, genomeSize;
		int* genome1; int* genome2;
		
		solution* child1 = new solution(townsNo, towns);
		solution* child2 = new solution(townsNo, towns);

		//Prepare children
		for (i = 0; i < townsNo; i++)
		{
			child1->townsOrder[i] = -1;
			child2->townsOrder[i] = -1;
		}

		//Ensure r1 is always smaller
		if (r1 > r2)
			swap(r2, r1);

		genomeSize = r2 - r1;

		genome1 = new int[genomeSize];
		genome2 = new int[genomeSize];

		//Fill genomes with values and children with genomes
		for (i = 0; i < genomeSize; i++)
		{
			genome1[i] = townsOrder[r1 + i];
			genome2[i] = parent->townsOrder[r1 + i];

			child1->townsOrder[r1 + i] = genome2[i];
			child1->townsOrder[r1 + i] = genome2[i];
		}

		//Fill children with parents genes that aren't found in genomes
		for (i = 0; i < townsNo; i++)
		{
			if (!isInIntArray(genome2, genomeSize, townsOrder[i]))
				child1->townsOrder[i] = townsOrder[i];
			if (!isInIntArray(genome1, genomeSize, parent->townsOrder[i]))
				child2->townsOrder[i] = parent->townsOrder[i];
		}

		//Fill children's remaining values
		for (i = 0; i < townsNo; i++)
		{
			//1st child case
			if (child1->townsOrder[i] == -1)
			{
				for (int j = 0; j < townsNo; j++)
				{
					if (!isInIntArray(child1->townsOrder, townsNo, parent->townsOrder[j]))
					{
						child1->townsOrder[i] = parent->townsOrder[j];
						break;
					}
				}
			}

			//2nd child case
			if (child2->townsOrder[i] == -1)
			{
				for (int j = 0; j < townsNo; j++)
				{
					if (!isInIntArray(child2->townsOrder, townsNo, townsOrder[j]))
					{
						child2->townsOrder[i] = townsOrder[j];
						break;
					}
				}
			}
		}

		//Exchange town orders
		for (i = 0; i < townsNo; i++)
		{
			swap(child1->townsOrder[i], townsOrder[i]);
			swap(child2->townsOrder[i], parent->townsOrder[i]);
		}

		//Set new evaluation values for both
		evaluate();
		parent->evaluate();

		//Free unused memory
		delete[] child1->townsOrder;
		delete[] child2->townsOrder;
		delete[] genome1;
		delete[] genome2;
		delete child1;
		delete child2;
	}

	void mutate()
	{
		//Get a substring of towns borders r1 and r2
		int r1 = rand() % townsNo, r2 = rand() % townsNo, i = 0;

		//Ensure r1 is always smaller
		if (r1 > r2)
			swap(r2, r1);
		
		//Reverse substring of order
		for (; i < (r2 - r1) / 2; i++)
			swap(townsOrder[r1 + i], townsOrder[r2 - i]);

		//Set new evaluation value
		evaluate();
	}

	// Only for debugging issues
	bool hasDuplicates()
	{
		for (int i = 0; i < townsNo; i++)
		{
			for (int j = i + 1; j < townsNo; j++)
			{
				if (townsOrder[i] == townsOrder[j])
					return true;
			}
		}

		return false;
	}

	// Only for debugging issues
	string toString()
	{
		string result = "Order of towns: ";

		for (unsigned int i = 0; i < townsNo; i++)
			result += to_string(townsOrder[i]) + ",";

		//Delete last unnecessary comma
		result = result.substr(0, result.size() - 1);

		result += "\nEvaluation value: " + to_string(evaluationValue);
		
		result += "\nHas duplicates: " + to_string(hasDuplicates()) + "\n";

		return  result;
	}
};

struct population
{
	int individualsNo = 0, townsNo = 0, bestIndivsNo = 5;
	solution best;
	town* towns;
	solution* individuals;

	population() {};

	population(int numberOfIndividuals, int numberOfTowns, town* towns) :
		individualsNo(numberOfIndividuals), townsNo(numberOfTowns), towns(towns)
	{
		individuals = new solution[numberOfIndividuals];
		
		populate();

		//Initialize and find best
		best = individuals[0];
		updateBest();
	}

	void reinitialize()
	{
		populate();

		//Initialize and find best
		best = individuals[0];
		updateBest();
	}

	inline void populate()
	{
		//Fill population with solutions
		int i = 0;
		
		for (; i < individualsNo; i++)
			individuals[i] = solution(townsNo, towns);
	}

	void updateBest()
	{
		int i = 0;

		for (; i < individualsNo; i++)
		{
			//If indiv's[i] townsOrder is shorter than best's townsOrder then replace best
			if (individuals[i].evaluationValue < best.evaluationValue)
				best = individuals[i];
		}

		//Allocate best's townsOrder so it persists after individual's townsOrder change
		for (i = 0; i < townsNo; i++)
			best.townsOrder[i] = int(best.townsOrder[i]);
	}

	void crossPopulation(int cChance)
	{
		int i = 0, winner;
		int* tournamentGroup = new int[bestIndivsNo];

		//Prepare group
		for (; i < bestIndivsNo; i++)
			tournamentGroup[i] = -1;

		//Find parents by tournament selection, place them at the beggining and cross
		for (; i < individualsNo - bestIndivsNo; i += 2)
		{
			//Find 1st parent
			tournamentGroup = fillTournamentGroup(i, tournamentGroup);
			winner = findTournamentWinner(tournamentGroup);
			swap(individuals[i], individuals[winner]);

			//Find 2nd parent
			tournamentGroup = fillTournamentGroup(i + 1, tournamentGroup);
			winner = findTournamentWinner(tournamentGroup);
			swap(individuals[i], individuals[winner]);

			//Cross parents with cChance percent chance
			if(rand() % 100 <= cChance)
				individuals[i].pmxCross(&individuals[i + 1]);
		}

		//Free unused memory
		delete[] tournamentGroup;

		//Update best after eventual changes
		updateBest();
	}

	inline int* fillTournamentGroup(int nextParticipantNo, int* group)
	{
		int i = 0, random;

		//Fill group with unique values
		for (i = 0; i < bestIndivsNo;)
		{
			random = nextParticipantNo + rand() % (individualsNo-nextParticipantNo);

			if (!isInIntArray(group, bestIndivsNo, random))
				group[i++] = random;
		}

		return group;
	}

	inline int findTournamentWinner(int* group)
	{
		int i = 1, winner = group[0];

		for (; i < bestIndivsNo; i++)
		{
			if (individuals[group[i]].evaluationValue < individuals[winner].evaluationValue)
				winner = group[i];
		}

		return winner;
	}

	void mutatePopulation(int mChance)
	{
		int i = 0;

		//Mutate individual with mChance percent chance
		for (; i < individualsNo; i++)
		{
			if (rand() % 100 <= mChance)
				individuals[i].mutate();
		}

		//Update best after eventual changes
		updateBest();
	}

	//Only for debgging issues
	string toString()
	{
		int i = 0;

		string result = "Population:\n";

		for (; i < individualsNo; i++)
		{
			result += individuals[i].toString();
		}

		result += "Populations best: \n" + best.toString();

		return result;
	}
};

void fillTownsEli51(town* towns)
{
	towns[0].setData(1, 37, 52);
	towns[1].setData(2, 49, 49);
	towns[2].setData(3, 52, 64);
	towns[3].setData(4, 20, 26);
	towns[4].setData(5, 40, 30);
	towns[5].setData(6, 21, 47);
	towns[6].setData(7, 17, 63);
	towns[7].setData(8, 31, 62);
	towns[8].setData(9, 52, 33);
	towns[9].setData(10, 51, 21);

	towns[10].setData(11, 42, 41);
	towns[11].setData(12, 31, 32);
	towns[12].setData(13, 5, 25);
	towns[13].setData(14, 12, 42);
	towns[14].setData(15, 36, 16);
	towns[15].setData(16, 52, 41);
	towns[16].setData(17, 27, 23);
	towns[17].setData(18, 17, 33);
	towns[18].setData(19, 13, 13);
	towns[19].setData(20, 57, 58);

	towns[20].setData(21, 62, 42);
	towns[21].setData(22, 42, 57);
	towns[22].setData(23, 16, 57);
	towns[23].setData(24, 8, 52);
	towns[24].setData(25, 7, 38);
	towns[25].setData(26, 27, 68);
	towns[26].setData(27, 30, 48);
	towns[27].setData(28, 43, 67);
	towns[28].setData(29, 58, 48);
	towns[29].setData(30, 58, 27);

	towns[30].setData(31, 37, 69);
	towns[31].setData(32, 38, 46);
	towns[32].setData(33, 46, 10);
	towns[33].setData(34, 61, 33);
	towns[34].setData(35, 62, 63);
	towns[35].setData(36, 63, 69);
	towns[36].setData(37, 32, 22);
	towns[37].setData(38, 45, 35);
	towns[38].setData(39, 59, 15);
	towns[39].setData(40, 5, 6);

	towns[40].setData(41, 10, 17);
	towns[41].setData(42, 21, 10);
	towns[42].setData(43, 5, 64);
	towns[43].setData(44, 30, 15);
	towns[44].setData(45, 39, 10);
	towns[45].setData(46, 32, 39);
	towns[46].setData(47, 25, 32);
	towns[47].setData(48, 25, 55);
	towns[48].setData(49, 48, 28);
	towns[49].setData(50, 56, 37);
	
	towns[50].setData(51, 30, 40);
}

void fillTownsBier127(town* towns)
{
	towns[0].setData(1, 9860, 14152);
	towns[1].setData(2, 9396, 14616);
	towns[2].setData(3, 11252, 14848);
	towns[3].setData(4, 11020, 13456);
	towns[4].setData(5, 9512, 15776);
	towns[5].setData(6, 10788, 13804);
	towns[6].setData(7, 10208, 14384);
	towns[7].setData(8, 11600, 13456);
	towns[8].setData(9, 11252, 14036);
	towns[9].setData(10, 10672, 15080);

	towns[10].setData(11, 11136, 14152);
	towns[11].setData(12, 9860, 13108);
	towns[12].setData(13, 10092, 14964);
	towns[13].setData(14, 9512, 13340);
	towns[14].setData(15, 10556, 13688);
	towns[15].setData(16, 9628, 14036);
	towns[16].setData(17, 10904, 13108);
	towns[17].setData(18, 11368, 12644);
	towns[18].setData(19, 11252, 13340);
	towns[19].setData(20, 10672, 13340);

	towns[20].setData(21, 11020, 13108);
	towns[21].setData(22, 11020, 13340);
	towns[22].setData(23, 11136, 13572);
	towns[23].setData(24, 11020, 13688);
	towns[24].setData(25, 8468, 11136);
	towns[25].setData(26, 8932, 12064);
	towns[26].setData(27, 9512, 12412);
	towns[27].setData(28, 7772, 11020);
	towns[28].setData(29, 8352, 10672);
	towns[29].setData(30, 9164, 12876);

	towns[30].setData(31, 9744, 12528);
	towns[31].setData(32, 8352, 10324);
	towns[32].setData(33, 8236, 11020);
	towns[33].setData(34, 8468, 12876);
	towns[34].setData(35, 8700, 14036);
	towns[35].setData(36, 8932, 13688);
	towns[36].setData(37, 9048, 13804);
	towns[37].setData(38, 8468, 12296);
	towns[38].setData(39, 8352, 12644);
	towns[39].setData(40, 8236, 13572);

	towns[40].setData(41, 9164, 13340);
	towns[41].setData(42, 8004, 12760);
	towns[42].setData(43, 8584, 13108);
	towns[43].setData(44, 7772, 14732);
	towns[44].setData(45, 7540, 15080);
	towns[45].setData(46, 7424, 17516);
	towns[46].setData(47, 8352, 17052);
	towns[47].setData(48, 7540, 16820);
	towns[48].setData(49, 7888, 17168);
	towns[49].setData(50, 9744, 15196);

	towns[50].setData(51, 9164, 14964);
	towns[51].setData(52, 9744, 16240);
	towns[52].setData(53, 7888, 16936);
	towns[53].setData(54, 8236, 15428);
	towns[54].setData(55, 9512, 17400);
	towns[55].setData(56, 9164, 16008);
	towns[56].setData(57, 8700, 15312);
	towns[57].setData(58, 11716, 16008);
	towns[58].setData(59, 12992, 14964);
	towns[59].setData(60, 12412, 14964);

	towns[60].setData(61, 12296, 15312);
	towns[61].setData(62, 12528, 15196);
	towns[62].setData(63, 15312, 6612);
	towns[63].setData(64, 11716, 16124);
	towns[64].setData(65, 11600, 19720);
	towns[65].setData(66, 10324, 17516);
	towns[66].setData(67, 12412, 13340);
	towns[67].setData(68, 12876, 12180);
	towns[68].setData(69, 13688, 10904);
	towns[69].setData(70, 13688, 11716);

	towns[70].setData(71, 13688, 12528);
	towns[71].setData(72, 11484, 13224);
	towns[72].setData(73, 12296, 12760);
	towns[73].setData(74, 12064, 12528);
	towns[74].setData(75, 12644, 10556);
	towns[75].setData(76, 11832, 11252);
	towns[76].setData(77, 11368, 12296);
	towns[77].setData(78, 11136, 11020);
	towns[78].setData(79, 10556, 11948);
	towns[79].setData(80, 10324, 11716);

	towns[80].setData(81, 11484, 9512);
	towns[81].setData(82, 11484, 7540);
	towns[82].setData(83, 11020, 7424);
	towns[83].setData(84, 11484, 9744);
	towns[84].setData(85, 16936, 12180);
	towns[85].setData(86, 17052, 12064);
	towns[86].setData(87, 16936, 11832);
	towns[87].setData(88, 17052, 11600);
	towns[88].setData(89, 13804, 18792);
	towns[89].setData(90, 12064, 14964);

	towns[90].setData(91, 12180, 15544);
	towns[91].setData(92, 14152, 18908);
	towns[92].setData(93, 5104, 14616);
	towns[93].setData(94, 6496, 17168);
	towns[94].setData(95, 5684, 13224);
	towns[95].setData(96, 15660, 10788);
	towns[96].setData(97, 5336, 10324);
	towns[97].setData(98, 812, 6264);
	towns[98].setData(99, 14384, 20184);
	towns[99].setData(100, 11252, 15776);

	towns[100].setData(101, 9744, 3132);
	towns[101].setData(102, 10904, 3480);
	towns[102].setData(103, 7308, 14848);
	towns[103].setData(104, 16472, 16472);
	towns[104].setData(105, 10440, 14036);
	towns[105].setData(106, 10672, 13804);
	towns[106].setData(107, 1160, 18560);
	towns[107].setData(108, 10788, 13572);
	towns[108].setData(109, 15660, 11368);
	towns[109].setData(110, 15544, 12760);

	towns[110].setData(111, 5336, 18908);
	towns[111].setData(112, 6264, 19140);
	towns[112].setData(113, 11832, 17516);
	towns[113].setData(114, 10672, 14152);
	towns[114].setData(115, 10208, 15196);
	towns[115].setData(116, 12180, 14848);
	towns[116].setData(117, 11020, 10208);
	towns[117].setData(118, 7656, 17052);
	towns[118].setData(119, 16240, 8352);
	towns[119].setData(120, 10440, 14732);

	towns[120].setData(121, 9164, 15544);
	towns[121].setData(122, 8004, 11020);
	towns[122].setData(123, 5684, 11948);
	towns[123].setData(124, 9512, 16472);
	towns[124].setData(125, 13688, 17516);
	towns[125].setData(126, 11484, 8468);
	towns[126].setData(127, 3248, 14152);
}

void enterUserData(/*int* indivsPerPopulation,*/ int* crossChance,
	int* mutationChance, int* iterationsNumber, int* numberOfIndividuals,
	int* dataSetNumber)
{
	//cout << "Enter target number of individuals per population (int):\n>";
	//cin >> *indivsPerPopulation;

	cout << "Enter cross chance % (int):\n>";
	cin >> *crossChance;

	cout << "Enter mutation chance % (int):\n>";
	cin >> *mutationChance;

	cout << "Enter iterations number (int):\n>";
	cin >> *iterationsNumber;

	cout << "Enter number of individuals (int)\n>";
	cin >> *numberOfIndividuals;

	cout << "Enter data set number (0 for Eli, 1 for Bier):\n>";
	cin >> *dataSetNumber;
}

int main()
{
	//Setting seed for different randoms
	srand((unsigned int)time(NULL));

	//Some variables are initialized so they don't have to be entered by enterUserData
	int numberOfPopulations = 0, targetNumberOfIndividuals = 10, townsNumber = 51,
		mutationChancePercent = 5, crossChancePercent = 95, iterationsNumber = 50000,
		numberOfIndividuals = 99, dataSetNumber = 0;

	int i = 0, j = 0, n, threadsNum;

	town* towns;
	population* populations;
	solution best;

	clock_t start, end;

	enterUserData(&crossChancePercent, &mutationChancePercent, &iterationsNumber, &numberOfIndividuals, &dataSetNumber);


	//Fill towns
	if (dataSetNumber)
	{
		townsNumber = 127;
		towns = new town[townsNumber];
		fillTownsBier127(towns);
	}
	else
	{
		towns = new town[townsNumber];
		fillTownsEli51(towns);
	}

	//====Solution struct test====
	//Solution init test
	/*
	solution s = solution(townsNumber, towns);
	cout << s.toString();
	*/
	//Solution cross test
	/*
	solution* s1 = new solution(townsNumber, towns);
	solution* s2 = new solution(townsNumber, towns);
	cout << s1->toString();
	cout << s2->toString();
	s1->pmxCross(s2);
	cout << s2->toString();
	cout << s1->toString();
	*/
	//Solution mutation test
	/*
	solution s = solution(townsNumber, towns);
	cout << s.toString();
	s.mutate(towns);
	cout << s.toString();
	*/
	//====Population struct test====
	//Population init test
	/*
	population p = population(targetNumberOfIndividuals, townsNumber, towns);
	cout << p.toString();
	*/
	//Population reinit test
	/*
	population p = population(targetNumberOfIndividuals, townsNumber, towns);
	cout << p.toString();
	p.reinitialize();
	cout << p.toString();
	*/
	//Population updateBest test
	/*
	population p = population(targetNumberOfIndividuals, townsNumber, towns);
	cout << p.toString();
	cout << p.best.toString();
	*/
	//Population mutate test
	/*
	population p = population(targetNumberOfIndividuals, townsNumber, towns);
	cout << p.toString();
	p.mutatePopulation(mutationChancePercent);
	cout << p.toString();
	*/
	//Population cross test
	/*
	population p = population(targetNumberOfIndividuals, townsNumber, towns);
	cout << p.toString() << endl << endl << endl;
	p.crossPopulation(crossChancePercent);
	cout << p.toString();
	*/



	//============================1 thread============================
	threadsNum = 1;
	omp_set_num_threads(threadsNum);

	n = numberOfIndividuals;
	
	if (numberOfIndividuals%targetNumberOfIndividuals)
		numberOfPopulations = (numberOfIndividuals / targetNumberOfIndividuals) + 1;
	else
		numberOfPopulations = numberOfIndividuals / targetNumberOfIndividuals;

	populations = new population[numberOfPopulations];

	//Fill populations
	while(n > 0)
	{
		if(n >= targetNumberOfIndividuals)
		{ 
			populations[i] = population(targetNumberOfIndividuals, townsNumber, towns);
			n -= targetNumberOfIndividuals;
		}
		else
		{
			populations[i] = population(n, townsNumber, towns);
			n = 0;
		}

		i++;
	}

	start = clock();

	//Begin evolutionary algorithm
	#pragma omp parallel default(shared) private (i, j)
	{
		#pragma omp for
		for (i = 0; i < numberOfPopulations; i++)
		{
			#pragma omp critical
			cout << "Population " << i << " is being processed by thread number " << omp_get_thread_num() <<".\n";

			for (j = 0; j < iterationsNumber; j++)
			{
				populations[i].crossPopulation(crossChancePercent);
				populations[i].mutatePopulation(mutationChancePercent);
			}
		}
	}

	//Initialize best
	best = populations[0].best;

	//Find best among populations
	for (i = 1; i < numberOfPopulations; i++)
	{
		if (populations[i].best.evaluationValue < best.evaluationValue)
			best = populations[i].best;
	}

	end = clock();

	cout << "Task has been completed in " << end - start << " ms with " << threadsNum << " thread." << endl;
	cout << best.toString();
	cout << endl;

	//============================2 threads============================
	threadsNum = 2;
	omp_set_num_threads(threadsNum);

	//Reinitialize populations
	for (i = 0; i < numberOfPopulations; i++)
	{
		populations[i].reinitialize();
	}

	start = clock();

	//Begin evolutionary algorithm
	#pragma omp parallel default(shared) private (i, j)
	{
		#pragma omp for
		for (i = 0; i < numberOfPopulations; i++)
		{
			#pragma omp critical
			cout << "Population " << i << " is being processed by thread number " << omp_get_thread_num() << ".\n";

			for (j = 0; j < iterationsNumber; j++)
			{
				populations[i].crossPopulation(crossChancePercent);
				populations[i].mutatePopulation(mutationChancePercent);
			}
		}
	}

	//Initialize best
	best = populations[0].best;

	//Find best among populations
	for (i = 1; i < numberOfPopulations; i++)
	{
		if (populations[i].best.evaluationValue < best.evaluationValue)
			best = populations[i].best;
	}

	end = clock();

	cout << "Task has been completed in " << end - start << " ms with " << threadsNum << " thread." << endl;
	cout << best.toString();
	cout << endl;

	//============================4 threads============================
	threadsNum = 4;
	omp_set_num_threads(threadsNum);

	//Reinitialize populations
	for (i = 0; i < numberOfPopulations; i++)
	{
		populations[i].reinitialize();
	}

	start = clock();

	//Begin evolutionary algorithm
	#pragma omp parallel default(shared) private (i, j)
	{	
		#pragma omp for
		for (i = 0; i < numberOfPopulations; i++)
		{
			#pragma omp critical
			cout << "Population " << i << " is being processed by thread number " << omp_get_thread_num() << ".\n";

			for (j = 0; j < iterationsNumber; j++)
			{
				populations[i].crossPopulation(crossChancePercent);
				populations[i].mutatePopulation(mutationChancePercent);
			}
		}
	}

	//Initialize best
	best = populations[0].best;

	//Find best among populations
	for (i = 1; i < numberOfPopulations; i++)
	{
		if (populations[i].best.evaluationValue < best.evaluationValue)
			best = populations[i].best;
	}

	end = clock();

	cout << "Task has been completed in " << end - start << " ms with " << threadsNum << " thread." << endl;
	cout << best.toString();
	cout << endl;

	//============================6 threads============================
	threadsNum = 6;
	omp_set_num_threads(threadsNum);

	//Reinitialize populations
	for (i = 0; i < numberOfPopulations; i++)
	{
		populations[i].reinitialize();
	}

	start = clock();

	//Begin evolutionary algorithm
	#pragma omp parallel default(shared) private (i, j)
	{
		#pragma omp for
		for (i = 0; i < numberOfPopulations; i++)
		{
			#pragma omp critical
			cout << "Population " << i << " is being processed by thread number " << omp_get_thread_num() << ".\n";

			for (j = 0; j < iterationsNumber; j++)
			{
				populations[i].crossPopulation(crossChancePercent);
				populations[i].mutatePopulation(mutationChancePercent);
			}
		}
	}

	//Initialize best
	best = populations[0].best;

	//Find best among populations
	for (i = 1; i < numberOfPopulations; i++)
	{
		if (populations[i].best.evaluationValue < best.evaluationValue)
			best = populations[i].best;
	}

	end = clock();

	cout << "Task has been completed in " << end - start << " ms with " << threadsNum << " thread." << endl;
	cout << best.toString();
	cout << endl;

	system("PAUSE");

    return EXIT_SUCCESS;
}