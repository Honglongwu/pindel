#include <algorithm>
#include <fstream>
#include "bddata.h"

BDData::BDData() : 	m_currentWindow( &g_dummyChromosome, 0, 0 )
{
   m_breakDancerMask = NULL;

   //m_currentChrName = "";
}

bool isNumber(const std::string & input) 
{
   for (unsigned int index = 0; index < input.size(); index++) {
		if (input[ index ]<'0' || input[ index ]>'9') return false;
	}	
    return true;
}


bool AtLeast6Fields(const std::string & input) {
	unsigned NumSpaceFields = 0;
	bool InSpace = false;
	unsigned InputSize = input.size();
	if (input[0] == '\t' || input[0] == ' ') return false; // must start with non-space char.
	for (unsigned index = 1; index < InputSize; index++) {
		if (input[index] == '\t' || input[index] == ' ') { // if current char is space, set InSpace to true
		InSpace = true;
		}
		else if (InSpace == true) { // not a space, NumSpaceFields++ if previous char is a space
			NumSpaceFields++;
			InSpace = false;
		}
	}
	if (NumSpaceFields >= 5) return true;
	else return false;
}


short CheckBreakDancerFileFormat(const std::string& filename) {

	std::ifstream CheckbdFileFirst( filename.c_str() );
	std::ifstream CheckbdFileSecond( filename.c_str() );
	char firstChar = 'k';
	std::string tempLine, errorLine;
	std::string Pos1, Pos2;
    
	while (!CheckbdFileFirst.eof()) {
		firstChar = CheckbdFileFirst.peek();// >> firstChar;
		if (firstChar == '#') {
			safeGetline(CheckbdFileFirst, tempLine);
          
			safeGetline(CheckbdFileSecond, tempLine);
		}
		else {
			safeGetline(CheckbdFileFirst, errorLine);
			//safeGetline(CheckbdFileSecond, errorLine);
			//std::cout << "errorLine/" << errorLine << "/" << std::endl;
           
            
			if (AtLeast6Fields(errorLine)) {
				//std::cout << "at least 6 fields" << std::endl;
				CheckbdFileSecond >> tempLine >> Pos1 >> tempLine
					>> tempLine >> Pos2 >> tempLine;

				safeGetline(CheckbdFileSecond, tempLine);  // get rest of line
				//std::cout << errorLine << " " << Pos1 << " " << Pos2 << std::endl;
				if (isNumber(Pos1) && isNumber(Pos2)) {
                	  
				}
				else {
					std::cout << "1 something is wrong with this line \"" << firstChar << errorLine << "\"" << std::endl;
					return 1;
				}
			}
			else if (errorLine != "") {
				std::cout << "2 something is wrong with this line \"" << firstChar << errorLine << "\"" << std::endl;
				return 1;
			}
		}
	}
    return 0;
}


void BDData::loadBDFile(const std::string& filename) {
	std::cout << " BDData::loadBDFile " << std::endl;
	if (CheckBreakDancerFileFormat(filename) == 1) {
		std::cout << "\nIgnore breakdancer file due to an error in the BreakDancer file format.\n" << std::endl;
		return; // if lines not start with #, there must be at least 6 fields and NO. 2 and No. 5 must be int.
	}
	std::ifstream bdFile( filename.c_str() );
	if (!bdFile.good()) {
		std::cout << "Error: cannot load breakdancer file '" << filename << std::endl;
		exit( EXIT_FAILURE );
	}

	char firstChar;
	std::string tempLine;

	while (!bdFile.eof()) {
		firstChar= bdFile.peek();
		if (firstChar == '#') {
		safeGetline(bdFile, tempLine);
		}
		else {
			std::string firstChrName, secondChrName;
			std::string tempStringItem;
			unsigned int firstPos, secondPos;
			bdFile >> firstChrName >> firstPos >> tempStringItem
				>> secondChrName >> secondPos >> tempStringItem;
			//std::cout << firstChrName << " " << firstPos << " " << tempStringItem << " " << secondChrName << " " << secondPos << std::endl;
			safeGetline(bdFile, tempLine);  // get rest of line

			firstPos += g_SpacerBeforeAfter; // ??? ask Kai
			secondPos += g_SpacerBeforeAfter;
            if (firstChrName == secondChrName && secondChrName != "" && abs(firstPos - secondPos) < 500)
                continue;
			if ( firstChrName!="" && secondChrName!="") {
				BreakDancerCoordinate firstBDCoordinate( firstChrName, firstPos );
				BreakDancerCoordinate secondBDCoordinate( secondChrName, secondPos ); 
	
				m_bdEvents_external.push_back(BreakDancerEvent( firstBDCoordinate, secondBDCoordinate ));
				m_bdEvents_external.push_back(BreakDancerEvent( secondBDCoordinate, firstBDCoordinate ));			
			}
		}
	}
	sort( m_bdEvents_external.begin(), m_bdEvents_external.end(), sortOnFirstBDCoordinate ); 
	std::cout << "BreakDancer events: " << m_bdEvents_external.size()/2 << std::endl;
}

bool Compare2Str (const std::string & first, const std::string & second) {
	if (first == second) return false;
	unsigned Length_first = first.size();
	unsigned Length_second = second.size();
	unsigned Length = std::min(Length_first, Length_second);
	for (unsigned index = 0; index < Length; index++) {
		if (first[index] > second[index]) return true;
		if (first[index] < second[index]) return false;
	}
	if (Length_first < Length_second) return true;
	else if (Length_first >= Length_second) return false;
	return false;
}

bool SortByFirstAndThenSecondCoordinate(const RP_READ & read1, const RP_READ & read2) {
	if (read1.PosA != read2.PosA ) {
		return (read1.PosA < read2.PosA );
	}
	else if (read1.PosB != read2.PosB ) {
		return (read1.PosB < read2.PosB );
	}
	else return false; // they're exactly equal so event1 is not 'smaller than' event2
}

/*
void SortRPByChrPos(std::vector <RP_READ> & Reads_RP) { // no interchromosome RP reads is here.
    //unsigned DistanceCutoff = 1000;
    //unsigned Num_RP_Reads = Reads_RP.size();
    
    sort(Reads_RP.begin(), Reads_RP.end(), SortByFirstAndThenSecondCoordinate);
}*/
bool RecipicalOverlap(RP_READ & first, RP_READ & second) {
	unsigned distance = 500;
	if (abs(first.PosA - first.PosA1) > distance) return false;
	if (abs(first.PosB - first.PosB1) > distance) return false;
	if (abs(second.PosA - second.PosA1) > distance) return false;
	if (abs(second.PosB - second.PosB1) > distance) return false;
    float cutoff = 0.9;
    
    unsigned FirstPosA = (first.PosA + first.PosA1)/2;
    unsigned FirstPosB = (first.PosB + first.PosB1)/2;
    if (FirstPosA > FirstPosB) std::swap(FirstPosA, FirstPosB);
    
    unsigned SecondPosA = (second.PosA + second.PosA1)/2;
    unsigned SecondPosB = (second.PosB + second.PosB1)/2;
    if (SecondPosA > SecondPosB) std::swap(SecondPosA, SecondPosB);
    
	if (first.DA != second.DA || first.DB != second.DB) return false;
	if (FirstPosA > SecondPosB + 200 || FirstPosB + 200 < SecondPosA) return false;
	if (FirstPosA <= SecondPosA && SecondPosB <= FirstPosB) {
		if ((double)(SecondPosB - SecondPosA)/(double)(FirstPosB - FirstPosA) >= cutoff) return true;
	}
	if (SecondPosA <= FirstPosA && FirstPosB <= SecondPosB) {
		if ((double)(FirstPosB - FirstPosA)/(double)(SecondPosB - SecondPosA) >= cutoff) return true;
	}
	if (FirstPosA <= SecondPosA && SecondPosA <= FirstPosB && FirstPosB <= SecondPosB) {
		if ((double)(FirstPosB - SecondPosA) / (double)(FirstPosB - FirstPosA) >= cutoff && (double)(FirstPosB - SecondPosA) / (double)(SecondPosB - SecondPosA) >= cutoff) return true;
	}
	if (SecondPosA <= FirstPosA && FirstPosA <= SecondPosB && SecondPosB <= FirstPosB) {
		if ((double)(SecondPosB - FirstPosA) / (double)(FirstPosB - FirstPosA) >= cutoff && (double)(SecondPosB - FirstPosA) / (double)(SecondPosB - SecondPosA) >= cutoff) return true;
	}
	return false;
    
}

bool Compare2RP (const RP_READ & first, const RP_READ & second) {
    if (first.OriginalPosA > second.OriginalPosA) {
        return true;
    }
    else if (first.OriginalPosA == second.OriginalPosA) {
        if (first.OriginalPosB > second.OriginalPosB) {
            return true;
        }
        else return false;
    }
    else return false;
    
}


void ModifyRP(std::vector <RP_READ> & Reads_RP) {
	std::cout << "Reads_RP.size(): " << Reads_RP.size() << std::endl;    

	if (Reads_RP.size() == 0) return;
	std::cout << "sorting read-pair" << std::endl;
	sort(Reads_RP.begin(), Reads_RP.end(), Compare2RP);
	std::cout << "sorting read-pair finished." << std::endl;
    
	for (int first = 0; first < (int)Reads_RP.size(); first++) {	//Han(2013.06.17)
		Reads_RP[first].PosA1 = Reads_RP[first].PosA;	//Han(2013.06.17)
		Reads_RP[first].PosB1 = Reads_RP[first].PosB;	//Han(2013.06.17)
	}	//Han(2013.06.17)
	
	int DistanceCutoff = 1000;
	#pragma omp parallel default(shared)
	{
		#pragma omp for
		for (int first = 0; first < (int)Reads_RP.size(); first++) {
			//unsigned Range, PosA_start, PosA_end, PosB_start, PosB_end;
			unsigned Stop_Pos;
			RP_READ & Current_first = Reads_RP[first];
			//Range = (unsigned)(Current_first.InsertSize);
			//if (Current_first.PosA > Range)
			//	PosA_start = Current_first.PosA - Range;
			//else PosA_start = 1;

			//PosA_end = Current_first.PosA + Range;
			//if (Current_first.PosB > Range)
			//	PosB_start = Current_first.PosB - Range;
			//else PosB_start = 1;

			//PosB_end = Current_first.PosB + Range;
			//Stop_Pos = PosA_end + DistanceCutoff;
			//if (Stop_Pos < PosB_end + DistanceCutoff)
			unsigned BP_end = Current_first.PosA;
			if (BP_end < Current_first.PosB)
				BP_end = Current_first.PosB;
			Stop_Pos = BP_end + DistanceCutoff;
	
			int Start_Index_Second = (int)first - 100;
			if (Start_Index_Second < 0) Start_Index_Second = 0;
			//if (Current_first.PosA < 160000)
			//std::cout << Current_first.ChrNameA << "\t" << Current_first.PosA << "\t" << Current_first.PosB << std::endl; 
			for (unsigned second = 0; second < Reads_RP.size(); second++) {
				//if (first == (int)second) continue;
				RP_READ & Current_second = Reads_RP[second];
				//std::cout << Current_second.ChrNameA << "\t" << Current_second.PosA << "\t" << Current_second.PosB << std::endl; 
				//if (Current_second.PosA > Stop_Pos && Current_second.PosB > Stop_Pos)
				//    break;
				if (RecipicalOverlap(Current_first, Current_second) == false) continue;
				//if (Current_first.ChrNameA == Current_second.ChrNameA && Current_first.ChrNameB == Current_second.ChrNameB)
				{
					//if (Current_second.PosA > Stop_Pos && Current_second.PosB > Stop_Pos) break;
					//if (Current_first.DA == Current_second.DA && Current_first.DB == Current_second.DB)
					{
						//if (Current_second.OriginalPosA > PosA_start
						//	&& Current_second.OriginalPosA < PosA_end
						//	&& Current_second.OriginalPosB > PosB_start
						//	&& Current_second.OriginalPosB < PosB_end)
						{ // same cluster
							if (Current_first.DA == '+' && Current_first.PosA < Current_second.OriginalPosA) {
								Current_first.PosA = Current_second.OriginalPosA;
							}
                					if (Current_first.DA == '-' && Current_first.PosA > Current_second.OriginalPosA) {
                			                Current_first.PosA = Current_second.OriginalPosA;
                					}
							if (Current_first.DB == '+' && Current_first.PosB < Current_second.OriginalPosB) {
								Current_first.PosB = Current_second.OriginalPosB;
								//std::cout << "PosB " << Current_first.PosB << std::endl;
							}
							if (Current_first.DB == '-' && Current_first.PosB > Current_second.OriginalPosB) {
								Current_first.PosB = Current_second.OriginalPosB;
							}
							// A1 and B1
							if (Current_first.DA == '+' && Current_first.PosA1 > Current_second.OriginalPosA) {
								Current_first.PosA1 = Current_second.OriginalPosA;
							}
							if (Current_first.DA == '-' && Current_first.PosA1 < Current_second.OriginalPosA) {
								Current_first.PosA1 = Current_second.OriginalPosA;
							}
                            
							if (Current_first.DB == '+' && Current_first.PosB1 > Current_second.OriginalPosB) {
								Current_first.PosB1 = Current_second.OriginalPosB;
								//std::cout << "PosB " << Current_first.PosB << std::endl;
							}
							if (Current_first.DB == '-' && Current_first.PosB1 < Current_second.OriginalPosB) {
								Current_first.PosB1 = Current_second.OriginalPosB;
							}
						}
					}
				}
			}
		}
	}// #pragma omp parallel default(shared)
	for (unsigned first = 0; first < Reads_RP.size(); first++) {
		if (Reads_RP[first].DA == '+') {
			Reads_RP[first].PosA = Reads_RP[first].PosA + Reads_RP[first].ReadLength;
			Reads_RP[first].PosA1 = Reads_RP[first].PosA1 + Reads_RP[first].ReadLength;
            
        	}
		if (Reads_RP[first].DB == '+') {
                	Reads_RP[first].PosB = Reads_RP[first].PosB + Reads_RP[first].ReadLength;
			Reads_RP[first].PosB1 = Reads_RP[first].PosB1 + Reads_RP[first].ReadLength;
		}
		if (Reads_RP[first].ChrNameA == Reads_RP[first].ChrNameB && abs(Reads_RP[first].PosA - Reads_RP[first].PosB) < 500) Reads_RP[first].Visited = true;
		//std::cout << "Final: " << Reads_RP[first].ChrNameA << " " << Reads_RP[first].DA << " " << Reads_RP[first].PosA << "\t" << Reads_RP[first].ChrNameB << " " << Reads_RP[first].DB << " " << Reads_RP[first].PosB << std::endl;
	}
}

void ModifyRP_InterChr(std::vector <RP_READ> & Reads_RP) {
	//std::cout << "InterChr RP reads: " << Reads_RP.size() << std::endl;
	if (Reads_RP.size() == 0) return;
	//#pragma omp parallel default(shared)
	{
	//#pragma omp for
		for (int first = 0; first < (int)Reads_RP.size() - 1; first++) {
			//#pragma omp critical

			unsigned Range, PosA_start, PosA_end, PosB_start, PosB_end;//, Stop_Pos;
			RP_READ & Current_first = Reads_RP[first];
			Range = (unsigned)(Current_first.InsertSize);
			if (Current_first.PosA > Range)
				PosA_start = Current_first.PosA - Range;
			else PosA_start = 1;
			PosA_end = Current_first.PosA + Range;
			if (Current_first.PosB > Range)
				PosB_start = Current_first.PosB - Range;
			else PosB_start = 1;
			PosB_end = Current_first.PosB + Range;
			//Stop_Pos = PosA_end + DistanceCutoff;
			//if (Stop_Pos < PosB_end + DistanceCutoff)
			//    Stop_Pos = PosB_end + DistanceCutoff;
			for (unsigned second = 0; second < Reads_RP.size(); second++) {
				//if (first == (int)second) continue;
				RP_READ & Current_second = Reads_RP[second];
				if (Current_first.ChrNameA == Current_second.ChrNameA && Current_first.ChrNameB == Current_second.ChrNameB) {
					//if (Current_second.PosA > Stop_Pos && Current_second.PosB > Stop_Pos) break;
					if (Current_first.DA == Current_second.DA
						&& Current_first.DB == Current_second.DB) {
						if (Current_second.OriginalPosA > PosA_start
							&& Current_second.OriginalPosA < PosA_end
							&& Current_second.OriginalPosB > PosB_start
							&& Current_second.OriginalPosB < PosB_end) { // same cluster
 							if ((Current_first.DA == '+' && Current_first.PosA < Current_second.OriginalPosA)
 								|| (Current_first.DA == '-' && Current_first.PosA > Current_second.OriginalPosA)) {
								Current_first.PosA = Current_second.OriginalPosA;
								//std::cout << "PosA " << Current_first.PosA << std::endl;
							}
							if ((Current_first.DB == '+' && Current_first.PosB < Current_second.OriginalPosB)
								|| (Current_first.DB == '-' && Current_first.PosB > Current_second.OriginalPosB)) {
								Current_first.PosB = Current_second.OriginalPosB;
								//std::cout << "PosB " << Current_first.PosB << std::endl;
							}
						}
					}
				}
				else if (Current_first.ChrNameA == Current_second.ChrNameB && Current_first.ChrNameB == Current_second.ChrNameA) {
					//if (Current_second.PosA > Stop_Pos && Current_second.PosB > Stop_Pos) break;
					if (Current_first.DA == Current_second.DB
						&& Current_first.DB == Current_second.DA) {
						if (Current_second.OriginalPosB > PosA_start
							&& Current_second.OriginalPosB < PosA_end
							&& Current_second.OriginalPosA > PosB_start
							&& Current_second.OriginalPosA < PosB_end) { // same cluster
 							if ((Current_first.DA == '+' && Current_first.PosA < Current_second.OriginalPosB)
 								|| (Current_first.DA == '-' && Current_first.PosA > Current_second.OriginalPosB)) {
								Current_first.PosA = Current_second.OriginalPosB;
								//std::cout << "PosA " << Current_first.PosA << std::endl;
							}
							if ((Current_first.DB == '+' && Current_first.PosB < Current_second.OriginalPosA)
								|| (Current_first.DB == '-' && Current_first.PosB > Current_second.OriginalPosA)) {
								Current_first.PosB = Current_second.OriginalPosA;
								//std::cout << "PosB " << Current_first.PosB << std::endl;
							}
						}
					}
				}
			}
		}
	}// #pragma omp parallel default(shared)
	for (unsigned first = 0; first < Reads_RP.size(); first++) {
		if (Reads_RP[first].DA == '+') Reads_RP[first].PosA = Reads_RP[first].PosA + Reads_RP[first].ReadLength;
		if (Reads_RP[first].DB == '+') Reads_RP[first].PosB = Reads_RP[first].PosB + Reads_RP[first].ReadLength;
		//if (Reads_RP[first].ChrNameA == Reads_RP[first].ChrNameB && abs(Reads_RP[first].PosA - Reads_RP[first].PosB) < 500) Reads_RP[first].Visited = true;
		//std::cout << "Final: " << Reads_RP[first].ChrNameA << " " << Reads_RP[first].DA << " " << Reads_RP[first].PosA << "\t" << Reads_RP[first].ChrNameB << " " << Reads_RP[first].DB << " " << Reads_RP[first].PosB << std::endl;
	}
	//std::cout << "Leaving" << std::endl;
}

void Summarize(std::vector <RP_READ> & Reads_RP) {
	unsigned Cutoff = 5;
	std::vector <unsigned int> GoodIndex;
	if (Reads_RP.size() < 5) {
		for (unsigned int first = 0; first < Reads_RP.size(); first++)
			Reads_RP[first].Report = false;
		return;
	}
	else {
		for (unsigned int first = 0; first < Reads_RP.size() - 1; first++) {
			//if (Reads_RP[first].PosA < 160000)
			//std::cout << Reads_RP[first].ChrNameA << "\t" << Reads_RP[first].DA << " " << Reads_RP[first].PosA << " " << Reads_RP[first].DB << " " << Reads_RP[first].PosB << std::endl;
			if (Reads_RP[first].Visited == true) continue;
			Reads_RP[first].NumberOfIdentical = 1;
			Reads_RP[first].Visited = false;
			for (unsigned second = first + 1; second < Reads_RP.size(); second++) {
				if (Reads_RP[second].Visited == true) continue;
				if (//Reads_RP[first].ChrNameA == Reads_RP[second].ChrNameA
					//&& Reads_RP[first].ChrNameB == Reads_RP[second].ChrNameB
					Reads_RP[first].PosA == Reads_RP[second].PosA
					&& Reads_RP[first].PosB == Reads_RP[second].PosB
					&& Reads_RP[first].PosA1 == Reads_RP[second].PosA1
					&& Reads_RP[first].PosB1 == Reads_RP[second].PosB1
					&& Reads_RP[first].DA == Reads_RP[second].DA
					&& Reads_RP[first].DB == Reads_RP[second].DB) {
						Reads_RP[first].NumberOfIdentical++;
						Reads_RP[second].Visited = true;
				}
			}

		}
        
		{
			for (unsigned index_a = 0;  index_a < Reads_RP.size() - 1; index_a++) {
				//std::cout << "index_a " << index_a << std::endl;
				//std::cout << Reads_RP[index_a].DA << " " << Reads_RP[index_a].PosA << " " << Reads_RP[index_a].DB << " " << Reads_RP[index_a].PosB << std::endl;
				if (Reads_RP[index_a].Visited) continue; 
				//if (Reads_RP[GoodIndex[index_a]].PosB - Reads_RP[GoodIndex[index_a]].PosA > 1000000) {
				//	std::cout << Reads_RP[GoodIndex[index_a]].PosA << "\t" << Reads_RP[GoodIndex[index_a]].PosA1 << "\t" 
				//		<< Reads_RP[GoodIndex[index_a]].PosB << "\t" << Reads_RP[GoodIndex[index_a]].PosB1 << std::endl;
				//}
				for (unsigned index_b = index_a + 1;  index_b < Reads_RP.size(); index_b++) {
					//std::cout << "index_b " << index_b << std::endl;
					if (RecipicalOverlap(Reads_RP[index_a], Reads_RP[index_b])) {
						//if ()
						//if (Reads_RP[GoodIndex[index_b]].PosB - Reads_RP[GoodIndex[index_b]].PosA > 1000000) {
						//	std::cout << Reads_RP[GoodIndex[index_b]].PosA << "\t" << Reads_RP[GoodIndex[index_b]].PosA1 << "\t" 
						//		<< Reads_RP[GoodIndex[index_b]].PosB << "\t" << Reads_RP[GoodIndex[index_b]].PosB1 << std::endl;
						//}
						Reads_RP[index_a].NumberOfIdentical = Reads_RP[index_a].NumberOfIdentical + Reads_RP[index_b].NumberOfIdentical;
						Reads_RP[index_b].Visited = true;
						if ((Reads_RP[index_a].DA == '+' && Reads_RP[index_a].PosA < Reads_RP[index_b].PosA) 
							|| (Reads_RP[index_a].DA == '-' && Reads_RP[index_a].PosA > Reads_RP[index_b].PosA)) 
							Reads_RP[index_a].PosA = Reads_RP[index_b].PosA;
						if ((Reads_RP[index_a].DB == '+' && Reads_RP[index_a].PosB < Reads_RP[index_b].PosB) 
							|| (Reads_RP[index_a].DB == '-' && Reads_RP[index_a].PosB > Reads_RP[index_b].PosB)) 
							Reads_RP[index_a].PosB = Reads_RP[index_b].PosB;
                        
                        if ((Reads_RP[index_a].DA == '+' && Reads_RP[index_a].PosA1 > Reads_RP[index_b].PosA1)
							|| (Reads_RP[index_a].DA == '-' && Reads_RP[index_a].PosA1 < Reads_RP[index_b].PosA1))
							Reads_RP[index_a].PosA1 = Reads_RP[index_b].PosA1;
						if ((Reads_RP[index_a].DB == '+' && Reads_RP[index_a].PosB1 > Reads_RP[index_b].PosB1)
							|| (Reads_RP[index_a].DB == '-' && Reads_RP[index_a].PosB1 < Reads_RP[index_b].PosB1))
							Reads_RP[index_a].PosB1 = Reads_RP[index_b].PosB1;
                        
					} 
				}
				if (Reads_RP[index_a].NumberOfIdentical >= Cutoff) {
					//if (Reads_RP[index_a].PosA < 160000)
					//	std::cout << Reads_RP[index_a].ChrNameA << "\t" << Reads_RP[index_a].DA << " " << Reads_RP[index_a].PosA 
					//		<< " " << Reads_RP[index_a].DB << " " << Reads_RP[index_a].PosB << "\t" << Reads_RP[index_a].NumberOfIdentical << std::endl;
					Reads_RP[index_a].Report = true;
				}
				else Reads_RP[index_a].Report = false;
			}
		}
        
	}
	//std::cout << "leaving summarize" << std::endl;
}

void Summarize_InterChr(std::vector <RP_READ> & Reads_RP) {
    unsigned Cutoff = 5;
	if (Reads_RP.size()==0) return;
    for (unsigned int first = 0; first < Reads_RP.size() - 1; first++) {
        //std::cout << Reads_RP[first].DA << " " << Reads_RP[first].PosA << " " << Reads_RP[first].DB << " " << Reads_RP[first].PosB << std::endl;
        if (Reads_RP[first].Visited == true) continue;
        Reads_RP[first].NumberOfIdentical = 0;
        for (unsigned second = first + 1; second < Reads_RP.size(); second++) {
            if (Reads_RP[second].Visited == true) continue;
            if (Reads_RP[first].ChrNameA == Reads_RP[second].ChrNameA
                && Reads_RP[first].ChrNameB == Reads_RP[second].ChrNameB
                && Reads_RP[first].PosA == Reads_RP[second].PosA
                && Reads_RP[first].PosB == Reads_RP[second].PosB
                && Reads_RP[first].DA == Reads_RP[second].DA
                && Reads_RP[first].DB == Reads_RP[second].DB) {
                Reads_RP[first].NumberOfIdentical++;
                Reads_RP[second].Visited = true;
            }
        }
        if (Reads_RP[first].NumberOfIdentical >= Cutoff) {
            Reads_RP[first].Report = true;
            //std::cout << Reads_RP[first].ChrNameA << " " << Reads_RP[first].PosA << " " << Reads_RP[first].ChrNameB << " " << Reads_RP[first].PosB << std::endl;
        }
        else Reads_RP[first].Report = false;
    }
    //std::cout << "Leaving Summarize_InterChr" << std::endl;
}

void BDData::UpdateBD(ControlState & currentState) {
	
	std::ofstream RPoutputfile(userSettings->getRPOutputFilename().c_str(), std::ios::app);
	m_bdEvents = m_bdEvents_external;

	//UserDefinedSettings* userSettings = UserDefinedSettings::Instance();
	std::cout << "Discovery RP: " << currentState.Reads_RP_Discovery.size() << std::endl;
	//std::vector <unsigned> RP_index;
	sort(currentState.Reads_RP_Discovery.begin(), currentState.Reads_RP_Discovery.end(), SortByFirstAndThenSecondCoordinate);
	//SortRPByChrPos(currentState.Reads_RP_Discovery);
	std::cout << "sorting RP complete." << std::endl;
	ModifyRP(currentState.Reads_RP_Discovery);
	std::cout << "Modify RP complete." << std::endl;
	Summarize(currentState.Reads_RP_Discovery);

	std::cout << "adding BD from RP." << std::endl;
	if (currentState.Reads_RP_Discovery.size()) {
		#pragma omp parallel default(shared)
		{
			#pragma omp for
			for (int read_index = 0; read_index < (int)currentState.Reads_RP_Discovery.size(); read_index++) {
				if (currentState.Reads_RP_Discovery[read_index].Report == false) continue;
   
				std::string firstChrName = currentState.Reads_RP_Discovery[read_index].ChrNameA;
				std::string secondChrName = currentState.Reads_RP_Discovery[read_index].ChrNameB;
				unsigned int firstPos = currentState.Reads_RP_Discovery[read_index].PosA + g_SpacerBeforeAfter;
                unsigned int firstPos2 = currentState.Reads_RP_Discovery[read_index].PosA1 + g_SpacerBeforeAfter;	//Han(2013.06.17)
                if (firstPos > firstPos2) std::swap(firstPos, firstPos2);
                if (currentState.Reads_RP_Discovery[read_index].DA == '+') firstPos2 += 200;
                else firstPos -= 200;
				unsigned int secondPos  = currentState.Reads_RP_Discovery[read_index].PosB + g_SpacerBeforeAfter;
				unsigned int secondPos2  = currentState.Reads_RP_Discovery[read_index].PosB1 + g_SpacerBeforeAfter;	//Han(2013.06.17)
                if (secondPos > secondPos2) std::swap(secondPos, secondPos2);
                if (currentState.Reads_RP_Discovery[read_index].DB == '+') secondPos2 += 200;
                else secondPos -= 200;
				if ( firstChrName!="" && secondChrName!="" ) {
					BreakDancerCoordinate firstBDCoordinate( firstChrName, firstPos, firstPos2 );
					BreakDancerCoordinate secondBDCoordinate( secondChrName, secondPos, secondPos2 );
					#pragma omp critical
					{
						m_bdEvents.push_back(BreakDancerEvent( firstBDCoordinate, secondBDCoordinate ));
						m_bdEvents.push_back(BreakDancerEvent( secondBDCoordinate, firstBDCoordinate ));
               					RPoutputfile << firstChrName  << "\t" << firstPos - g_SpacerBeforeAfter  << "\t" << firstPos2 - g_SpacerBeforeAfter << "\t"  << currentState.Reads_RP_Discovery[read_index].DA << "\t" 
							     << secondChrName << "\t" << secondPos - g_SpacerBeforeAfter << "\t" << secondPos2 - g_SpacerBeforeAfter << "\t" << currentState.Reads_RP_Discovery[read_index].DB << "\t" 
							     << abs((int)secondPos - (int)firstPos) << "\tSupport: " << currentState.Reads_RP_Discovery[read_index].NumberOfIdentical << std::endl;
						std::cout << "adding " << firstChrName << " " << firstPos - g_SpacerBeforeAfter << "\t" << firstPos2 - g_SpacerBeforeAfter << "\t" << currentState.Reads_RP_Discovery[read_index].DA 
							<< "\t" << secondChrName << " " << secondPos - g_SpacerBeforeAfter << "\t" << secondPos2 - g_SpacerBeforeAfter << "\t" << currentState.Reads_RP_Discovery[read_index].DB 
							<< " to breakdancer events. " << abs((int)secondPos - (int)firstPos) << " Support: " << currentState.Reads_RP_Discovery[read_index].NumberOfIdentical << std::endl;
					}
				}
        
			}
		}
	}
	currentState.Reads_RP_Discovery.clear();

	std::cout << "modify and summarize interchr RP." << std::endl;
	if (userSettings->reportInterchromosomalEvents && currentState.Reads_RP_Discovery_InterChr.size()) {
		ModifyRP_InterChr(currentState.Reads_RP_Discovery_InterChr);
		Summarize_InterChr(currentState.Reads_RP_Discovery_InterChr);
	}

	std::cout << "adding BD from interChr RP." << std::endl;
	if (currentState.Reads_RP_Discovery_InterChr.size() && userSettings->reportInterchromosomalEvents) {
		#pragma omp parallel default(shared)
		{
			#pragma omp for
			for (int read_index = 0; read_index < (int)currentState.Reads_RP_Discovery_InterChr.size(); read_index++) {
				if (currentState.Reads_RP_Discovery_InterChr[read_index].Report == false) continue;
              
				std::string firstChrName = currentState.Reads_RP_Discovery_InterChr[read_index].ChrNameA;
				std::string secondChrName = currentState.Reads_RP_Discovery_InterChr[read_index].ChrNameB;

				unsigned int firstPos = currentState.Reads_RP_Discovery_InterChr[read_index].PosA + g_SpacerBeforeAfter;
				unsigned int secondPos  = currentState.Reads_RP_Discovery_InterChr[read_index].PosB + g_SpacerBeforeAfter;

				if ( firstChrName!="" && secondChrName!="" ) {
					BreakDancerCoordinate firstBDCoordinate( firstChrName, firstPos );
					BreakDancerCoordinate secondBDCoordinate( secondChrName, secondPos );

					#pragma omp critical
					{

						m_bdEvents.push_back(BreakDancerEvent( firstBDCoordinate, secondBDCoordinate ));

						m_bdEvents.push_back(BreakDancerEvent( secondBDCoordinate, firstBDCoordinate ));
						RPoutputfile << firstChrName << "\t" << firstPos - g_SpacerBeforeAfter  << "\t" << firstPos - g_SpacerBeforeAfter  << "\t"  << currentState.Reads_RP_Discovery_InterChr[read_index].DA << "\t" 
							    << secondChrName << "\t" << secondPos - g_SpacerBeforeAfter << "\t" << secondPos - g_SpacerBeforeAfter << "\t"  << currentState.Reads_RP_Discovery_InterChr[read_index].DB << "\t0\t" 
							    << "\tSupport: " << currentState.Reads_RP_Discovery_InterChr[read_index].NumberOfIdentical << std::endl;
						std::cout << "adding interchr " << firstChrName << " " << firstPos - g_SpacerBeforeAfter << "\t" << secondChrName << " " << secondPos - g_SpacerBeforeAfter <<  " to breakdancer events. " 								<< " Support: " << currentState.Reads_RP_Discovery_InterChr[read_index].NumberOfIdentical << std::endl;
					}

				}
                
			}
		}
	}
	currentState.Reads_RP_Discovery_InterChr.clear();
	//currentState.Reads_RP_Discovery.clear();
	std::cout << "summarize BP as BD complete. Now start sorting BD..." << std::endl;
	sort( m_bdEvents.begin(), m_bdEvents.end(), sortOnFirstBDCoordinate );
	std::cout << "sorting BD... done." << std::endl;
}


bool regionsOverlap( const BDIterator& firstRegionIt, const BDIterator& secondRegionIt )
{
	return ( ( secondRegionIt->second.getChromosomeName() == firstRegionIt->second.getChromosomeName() )
		&& ( secondRegionIt->second.startOfWindow() <= firstRegionIt->second.endOfWindow() + 1 ) );
}

void BDData::createRegionCluster(const BDIterator& startOfEventList, const BDIterator& endOfEventList, SearchWindowCluster& newCluster)
{
	std::vector<BreakDancerEvent> relevantSubcluster( startOfEventList, endOfEventList );
	sort( relevantSubcluster.begin(), relevantSubcluster.end(), sortOnSecondBDCoordinate );
		//std::cout << "Making cluster" << "\n";
	newCluster.clear();
    
    //for (BDIterator eventIter=relevantSubcluster.begin(); eventIter!=relevantSubcluster.end(); eventIter++ ) {	//Han(2013.06.17)
	//	std::cout << "Han : start " << eventIter->first.startOfWindow() - g_SpacerBeforeAfter << "\tpos:" << eventIter->first.position - g_SpacerBeforeAfter << "/pos2:" << eventIter->first.position2 - g_SpacerBeforeAfter << "\tend " << eventIter->first.endOfWindow() - g_SpacerBeforeAfter << "\t" << eventIter->second.startOfWindow() - g_SpacerBeforeAfter << "\tpos: " << eventIter->second.position - g_SpacerBeforeAfter << "/pos2" << eventIter->second.position2 - g_SpacerBeforeAfter << "\t" << eventIter->second.endOfWindow() - g_SpacerBeforeAfter << "\n";	//Han(2013.06.17)
	//}	//Han(2013.06.17)
    
	for (BDIterator eventIter=relevantSubcluster.begin(); eventIter!=relevantSubcluster.end(); ++eventIter ) {
		//std::cout << "FC: " << eventIter->first.getChromosomeName() << "FS: " << eventIter->first.startOfWindow() << "FE: " << eventIter->first.endOfWindow() << 
		//	"SC: " << eventIter->second.getChromosomeName() << "SS: " << eventIter->second.startOfWindow()<< "SE: " << eventIter->second.endOfWindow() << "\n";
		// NOTE: below code will be removed once we start working on interchromosomal translocations
		/*if (eventIter->second.getChromosomeName() != m_currentWindow.getChromosomeName() ) {
			std::cout << "Possible translocation from chromosome " << m_currentWindow.getChromosomeName() << " to chromosome " << eventIter->second.getChromosomeName() << "\n";
		}
		else {*/

			SearchWindow currentEventWindow( eventIter->second.getChromosome(), eventIter->second.startOfWindow(), eventIter->second.endOfWindow() );
			while ( eventIter+1!=relevantSubcluster.end() && regionsOverlap( eventIter, eventIter+1 ) ) {
				eventIter++;
				currentEventWindow.setEnd( eventIter->second.endOfWindow() );
			}
			//std::cout << "Pushing back\n";
			newCluster.push_back( currentEventWindow );
		//}
	}
}


void BDData::loadRegion( const SearchWindow& searchWindow  )
{
	// check how we handle borders/reads near borders at the moment
	//std::cout << "Starting loadRegion...\n";
	m_currentWindow = searchWindow;
	//std::cout << "m_currentWindow start " << m_currentWindow.getStart() << " end " << m_currentWindow.getEnd() << "size" << m_currentWindow.getSize()<< "\n";
	const int INSERT_SIZE = 1000; // will need to adjust this!
	if (m_currentWindow.getStart() >= 3*INSERT_SIZE) {
		m_currentWindow.setStart( m_currentWindow.getStart() - 3*INSERT_SIZE );
	}
	else {
		m_currentWindow.setStart( 0 ); 
	}
	//std::cout << "chromosomePtr=" << m_currentWindow.getChromosome() << "\n";	
	//std::cout << "m_currentWindow start " << m_currentWindow.getChromosome()->getName() << " z " << m_currentWindow.getStart() << " end " << m_currentWindow.getEnd() << "size" << m_currentWindow.getSize()<< "\n";
	/*for (unsigned int ki=0; ki<m_bdEvents.size(); ki++ ) {
		std::cout << m_bdEvents[ki].first.getChromosomeName() <<" " << m_bdEvents[ki].first.position << " - " << m_bdEvents[ki].second.position << "-";
		std::cout << m_bdEvents[ki].second.getChromosomeName() <<" " << m_bdEvents[ki].second.position << " - " << m_bdEvents[ki].second.position << "\n";
	}*/
	//std::cout << "X\n";
	//std::cout << "m_currentWindow startA " << m_currentWindow.getChromosome()->getName() << "\n";
	m_currentWindow.setEnd( m_currentWindow.getEnd() + 3*INSERT_SIZE );
	//std::cout << "m_currentWindow startB " << m_currentWindow.getChromosome()->getName() << "\n";
	BreakDancerCoordinate emptyBDCoord("",0);
	//std::cout << "m_currentWindow startC " << m_currentWindow.getChromosome()->getName() << "\n";
	//std::cout << "Y\n";
	BreakDancerCoordinate startOfWindowBDCoord( m_currentWindow.getChromosome(), m_currentWindow.getStart() );
	//std::cout << "Z\n";
	BDIterator startRegionInBDEvents = lower_bound( m_bdEvents.begin(), m_bdEvents.end(), BreakDancerEvent(startOfWindowBDCoord, emptyBDCoord), sortOnFirstBDCoordinate );
	//std::cout << "Z1\n";
	BreakDancerCoordinate endOfWindowBDCoord( m_currentWindow.getChromosome(), m_currentWindow.getEnd() );
	//std::cout << "Last coordinate: " << endOfWindowBDCoord.position << std::endl;
	BreakDancerEvent endEvent(endOfWindowBDCoord, emptyBDCoord);
	//std::cout << "Last coordinate(2): " << endEvent.first.position << std::endl;	
	BDIterator endRegionInBDEvents = upper_bound( m_bdEvents.begin(), m_bdEvents.end(), endEvent, sortOnFirstBDCoordinate ); 
	//std::cout << "The last is at..." <<   endRegionInBDEvents-m_bdEvents.begin() << "\n";;
	//std::cout<< "Investigating: " << endRegionInBDEvents->first.position << " - " << startRegionInBDEvents->first.position << "of" << m_bdEvents.end()-m_bdEvents.begin() << "\n"; 
	/*std::cout << "After selecting:\n";

	for (BDIterator it=startRegionInBDEvents; it!=endRegionInBDEvents; it++ ) {
		std::cout << it->first.getChromosomeName() <<" " << it->first.position << " - " << it->second.position << "\n";
	}*/


	delete[] m_breakDancerMask; // removing NULL is also safe
   m_breakDancerMask = new unsigned int[ m_currentWindow.getSize() ];

	SearchWindowCluster emptyCluster;	
	m_regionsToScanCollection.clear();
	m_regionsToScanCollection.push_back( emptyCluster );
	////std::cout << "Continuing LoadRegion\n";		
	BDIterator startOfEventList = startRegionInBDEvents;
	BDIterator endOfEventList = startRegionInBDEvents;
	int index = 0;
	
	for ( unsigned int position=m_currentWindow.getStart(); position< m_currentWindow.getEnd(); position++ ) {
		bool changed = false;
	////std::cout << "At position " << position << "\n";	

		// remove events that have been passed already
		for ( BDIterator eventIter=startOfEventList; eventIter<endOfEventList; ++eventIter ) {
			if ( position > eventIter->first.endOfWindow() ) {
				startOfEventList++;
				changed = true;
			}
			else { // <= to end position of event
				break;
			}
		}
	
		// add new events
		for (BDIterator eventIter=endOfEventList; eventIter<endRegionInBDEvents; ++eventIter ) {
			if ( position < eventIter->first.startOfWindow() ) {
				break;
			}
			else if ( position == eventIter->first.startOfWindow() ) {
                //std::cout << "I should be making an event now!\n";
				endOfEventList++;
				changed = true;
			}
		}
		// so if there are no events suitable, save 0 as the value
		if ( startOfEventList == endOfEventList ) {
			m_breakDancerMask[ position-m_currentWindow.getStart() ] = 0;
		}
		else {
			if (changed) {
				index++;
				SearchWindowCluster newCluster;
                                //std::cout << "I should be making a cluster now!\n";
				createRegionCluster( startOfEventList, endOfEventList, newCluster);
				m_regionsToScanCollection.push_back( newCluster );
                                //std::cout << "With " << newCluster.size() << "events\n";
			}
			m_breakDancerMask[ position-m_currentWindow.getStart() ] = index;
			//if (index%50==0) { //std::cout << "Mask making: position " << position << " has index " << index << "\n"; }
		}	
	}
}


const SearchWindowCluster& BDData::getCorrespondingSearchWindowCluster( const SPLIT_READ& read ) const {
	//std::cout << "entering getCorrespondingSearchWindowCluster " << std::endl;
	if ( read.getLastAbsLocCloseEnd() - m_currentWindow.getStart() < 0 || read.getLastAbsLocCloseEnd() - m_currentWindow.getStart() > m_currentWindow.getSize() ) {
		//std::cout << "Coordinate of the close end " << read.getLastAbsLocCloseEnd() << " is bad.\n";
		//std::cout << "Start: " << m_currentWindow.getStart() << " end " << m_currentWindow.getEnd() << " size " << m_currentWindow.getSize() << "\n";
		//SearchWindowCluster EmptyReturn;
		return m_regionsToScanCollection[ 0 ];
		//exit( EXIT_FAILURE );		
	}
	unsigned int clusterIndex = 0;
	//std::cout << read.getLastAbsLocCloseEnd() << "\t" << m_currentWindow.getStart() << std::endl;
	if (read.getLastAbsLocCloseEnd() > m_currentWindow.getStart()) 
		clusterIndex = m_breakDancerMask[ read.getLastAbsLocCloseEnd() - m_currentWindow.getStart() ];
	
	//std::cout << "leaving getCorrespondingSearchWindowCluster " << std::endl;
	return m_regionsToScanCollection[ clusterIndex ];
}

bool haveCommonBDEvent( const SearchWindowCluster& event, const unsigned int position, const std::string& chromosomeName )
{
	for (unsigned int index=0; index<event.size(); index++) {
		if (event[ index ].encompasses( chromosomeName, position )) {
			return true;	
		}
	}
	return false;
} 

/* 'isBreakdancerEvent' returns whether an event between leftPosition and rightPosition in the current chromosome
	is confirmed by a breakdancer result. */
bool BDData::isBreakDancerEvent( const unsigned int leftPosition, const unsigned int rightPosition ) const
{
   unsigned int rawLeftPosition = leftPosition + g_SpacerBeforeAfter;
   unsigned int rawRightPosition = rightPosition + g_SpacerBeforeAfter;

   if ( m_breakDancerMask[ rawLeftPosition - m_currentWindow.getStart() ]!=0  &&
		  m_breakDancerMask[ rawRightPosition - m_currentWindow.getStart() ]!=0 ) {
		//std::cout << "Found breakdancerEvent near " << leftPosition << "-" << rightPosition << "\n";
        
		return haveCommonBDEvent( m_regionsToScanCollection[ m_breakDancerMask[ rawLeftPosition - m_currentWindow.getStart() ] ],
				rawRightPosition - m_currentWindow.getStart(), m_currentWindow.getChromosomeName() );
   }
   else {
      return false;
   }
}

unsigned BDData::GetBDSize_external(){return m_bdEvents_external.size();}

unsigned BDData::GetBDSize_total(){return m_bdEvents.size();}

