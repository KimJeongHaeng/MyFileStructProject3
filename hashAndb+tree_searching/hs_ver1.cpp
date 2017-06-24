#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <list>
#include <algorithm>
#include <vector>

using namespace std;
/* run this program using the console pauser or add your own getch, system("pause") or input loop */

#define M 513  // 홀수 차수 생성 
#define TR 500 // 스택의 크기

//student's dataStruct
typedef struct InputData {
	char name[20];
	unsigned int studentID;
	float score;
	unsigned int advisorID;	
}StudentData;

//hashTable's node dataStruct
typedef struct Hashform {
	int pointBlockNum;
}HashData;

//BlockNode's dataStruct
typedef struct Blockform {
	int thisBlockNum;
	vector <StudentData> studentData;
}BlockData;

typedef struct NodeData{
	float score;
	int bNum;
}nData;

typedef struct Node
{
	int count;		// number of key	
	float Key[M-1];			 // Key 
	struct Node* branch[M];  // next node 
	StudentData* data;
} node;

class Bptree{
	public:
		bool insertItem(nData* k);		// Key insert
		void kSearch(int k);
		nData* getLeaves(int n); // get n leaf nodes 
	private:
		node* root;
};

class StudentsFileStruct {
	private:
		
		int studNum; 
		fstream hashIO;
		fstream DBIO;
		
		
		//이위로 새로짜는 코드 
		int testNum = 0;
		
		int studentNum; //student number
		int hashTablePrefix;
		//int hashTablePrefix; //hashTable's prefix
		StudentData *studentsData; //all data recods input
		vector <HashData> hashNode; // hashNodes is vector .. table type
		vector <HashData>::iterator hash_iter;
		vector <BlockData> blockNode; // blockNoes is vector
		vector <BlockData>::iterator block_iter;
		Bptree studentTree;
		nData *nodeData;
		
	public:
		void fileHashAndDBOpen() {
			
			ofstream hashIO_temp;
			ofstream DBIO_temp;
			
			hashIO_temp.open("Students.hash", ios::out | ostream::binary);
			hashIO_temp.close();
				
			hashIO.open("Students.hash", ios::in | ios::out | ostream::binary);
			
			hashIO.seekp(0);
			int firstHashTablePrefix = 0;
			int hashPointBlockNum = 0;
			hashIO.write((char*)(&firstHashTablePrefix), sizeof(firstHashTablePrefix));
			hashIO.write((char*)(&hashPointBlockNum), sizeof(hashPointBlockNum));
			
			DBIO_temp.open("Student.DB", ios::out | ostream::binary);
			DBIO_temp.close();
			
			DBIO.open("Student.DB", ios::in | ios::out | ostream::binary);
			
			//fo.write((char*)(&(blockNode[i].studentData[j].name)), sizeof(blockNode[i].studentData[j].name));
			//char a[20];
			//fi.read((char*)(&a), sizeof(a));
		}
		
		void readStudTableAndUpdateFile() { //자료들을 한줄씩 읽음 
			ifstream readStudFile("sampleData5.csv");
			//input student number by using getline
			string tmp_studentNum;
			getline(readStudFile, tmp_studentNum, '\n');
			studentNum = atoi(tmp_studentNum.c_str());
			
			StudentData currentStudData;
			int useHashingValue;
			int useHashingPrefix;
			int thisBlockNumber;
			//input all studentData by using getline
			
			string tmp_name, tmp_studentID, tmp_score, tmp_advisorID;
			while(getline(readStudFile, tmp_name,',')&&getline(readStudFile, tmp_studentID,',')&&getline(readStudFile, tmp_score,',')&&getline(readStudFile,tmp_advisorID,'\n')) {
				strcpy(currentStudData.name,tmp_name.c_str());
				if(tmp_name.length() >= 20) {
					string tmp_re_name = tmp_name.substr(0, 19);
					strcpy(currentStudData.name,tmp_re_name.c_str());
				}
				currentStudData.studentID = atoi(tmp_studentID.c_str());
				currentStudData.score = atof(tmp_score.c_str());
				currentStudData.advisorID = atoi(tmp_advisorID.c_str());
				
				hashIO.clear();
				hashIO.seekg(0);
				hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
				//useHashingPrefix = useHashingPrefix;
				
				useHashingValue = findHashValue(currentStudData.studentID, useHashingPrefix);
				thisBlockNumber = findBlockNumber(useHashingValue);
				
				inputDataInDB(currentStudData, thisBlockNumber);
				//cout << useHashingValue << " ";
				//studentTree.insertItem(studentsData+dataLocate);
				//cout << studentsData[dataLocate].name << "," << studentsData[dataLocate].studentID << "," << studentsData[dataLocate].score << "," << studentsData[dataLocate].advisorID << endl;
				
			}
			
			readStudFile.close();
			hashIO.close();
			DBIO.close();
			
		}
		
		int findHashValue(unsigned int currentStudData_ID, int useHashingPrefix) { // 헤쉬테이블의 prefix이용 단, hashvalue가 "",0인경우는 같은 형식으로 취급 
			//hash파일과 DB파일은 계속 열려있어야함, hash파일과 DB파일이 update해주어야 함
			string hashingValue_string = hashingFunction(currentStudData_ID);
			//useHashingPrefix = 1;
			if(hashingValue_string.length() < useHashingPrefix) {
				while(hashingValue_string.length() != useHashingPrefix) {
					hashingValue_string = "0" + hashingValue_string;
				}
			}
			string useHashingValue_string;
			int useHashingValue_int;
			
			if(!hashingValue_string.compare("") == 0) {
				useHashingValue_string = hashingValue_string.substr(hashingValue_string.length() - useHashingPrefix, useHashingPrefix);
				useHashingValue_int = binaryToDecimal(useHashingValue_string);
			} else {
				useHashingValue_int = -1;
			}
			
			//prefix보고 substring추출
			 
			/*int i = 0;
			hashIO.seekp(0);
			hashIO.write((char*)(&i), sizeof(i));
			*/
			/*int j;
			hashIO.seekg(0);
			hashIO.read((char*)(&j), sizeof(j));
			
			cout << j << " ";*/
			
			//cout << currentStudData.name << endl;
			
			return useHashingValue_int;
		}
		
		string hashingFunction(unsigned int studentID) {
			
			if(studentID == 0) {
				return "0";
			} else if (studentID== 1) {
				return "1";
			} else if (studentID % 2 == 0) {
				return hashingFunction(studentID / 2) + "0";
			} else {
				return hashingFunction(studentID / 2) + "1";
			}
			
		}
		
		int binaryToDecimal(string useHashingValue_string) {
			int useHashingValue_int = 0, pow = 1;
			for(int i = useHashingValue_string.length() - 1 ;i >= 0 ; i--, pow <<= 1) {
				useHashingValue_int += (useHashingValue_string[i] - '0') * pow;
			}
			return useHashingValue_int;
		}
		
		int findBlockNumber(int useHashingValue) {
			int thisBlockNumber = 0;
			
			if(useHashingValue == -1) {
				useHashingValue == 0;
			}
			hashIO.clear();
			hashIO.seekg(4 + (4 * useHashingValue));
			hashIO.read((char*)(&thisBlockNumber), sizeof(thisBlockNumber));
			//cout << thisBlockNumber;
			return thisBlockNumber;
		}
		
		void inputDataInDB(StudentData currentStudData, int thisBlockNumber) {
			//cout << currentStudData.name << " ";
			//cout << sizeof(currentStudData) << " ";
			bool isOverflow = overflowCheck(thisBlockNumber, sizeof(currentStudData));
			
			if(!isOverflow) {
				inputData(currentStudData, thisBlockNumber);
				//cout << testNum << " ";
				//testNum++;
			} else {
				solutionOverflow(currentStudData, thisBlockNumber);
			}
		}
		
		bool overflowCheck(int thisBlockNumber, int dataSize) {
			
			bool isOverflow = true;
			
			char isNull;
			DBIO.clear();
			DBIO.seekg((thisBlockNumber + 1) * 4096 - dataSize);
			
			DBIO.read((char*)(&isNull), sizeof(isNull));
			
			if(isNull == NULL || DBIO.tellg() == -1) {
				isOverflow = false;
			}
			/*for(int i = 0; i < 4096; i += dataSize) {
				hashIO.read((char*)(&isNull), sizeof(isNull));
				if(isNull == NULL) {
					isOverflow = true;
					break;
				}
			}*/
			
			return isOverflow;
		}
		
		void inputData(StudentData currentStudData, int thisBlockNumber) {			
			char isNull;
			int inputLocation = 0;
			
			for(int i = 0; i < 4096; i += sizeof(currentStudData)) {
			//for(int i = 0; i < 4096; i ++) {
				DBIO.clear();
				DBIO.seekg((thisBlockNumber * 4096) + i);
				DBIO.read((char*)(&isNull), sizeof(isNull));
				//cout << isNull << "구분자";
				
				if(isNull == NULL || DBIO.tellg() == -1) {
					inputLocation = i;
					//cout << i << " ";
					break;
				}
			}
			
			//cout <<
			DBIO.clear();
			DBIO.seekp(thisBlockNumber * 4096 + inputLocation);
			//cout << currentStudData.name << " ";
			DBIO.write((char*)(&currentStudData.name), sizeof(currentStudData.name));
			DBIO.write((char*)(&currentStudData.studentID), sizeof(currentStudData.studentID));
			DBIO.write((char*)(&currentStudData.score), sizeof(currentStudData.score));
			DBIO.write((char*)(&currentStudData.advisorID), sizeof(currentStudData.advisorID));
			
		}
		
		void solutionOverflow(StudentData currentStudData, int thisBlockNumber) {
			//hashtable에서 현재 blocknumber와 같은것 찾음 - 함수만들기 
			int blockPointCount = findBlockPointCount(thisBlockNumber);
			//cout << blockPointCount << " ";
			//1개면 hashtable을 두배로 늘린뒤 데이터를 다 들고와서 hashPrefix재설정후(prefix1증가) 
			//block을 하나 더 만들어서 재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow를 확인후(재귀호출) 현재 데이터 삽입
			if(blockPointCount == 1) {
				reallocationOnePoint(currentStudData, thisBlockNumber); 
			} else {
				//2개, 4개, 8개 ...면 hashtable을 나눌 필요없이 hashPrefix를 적절히 보고  block을 늘릴 필요없이 데이터를 다 들고와서
				//재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
				//다시 넣을 곳의 block의 overflow확인후 (재귀호출 이때는 point가 한개가 될수있음) 현재 데이터 삽입 
				reallocationMultiPoint(currentStudData, thisBlockNumber);
			}
			
			
		}
		
		int findBlockPointCount(int thisBlockNumber) {
			int blockPointCount = 0;
			int useHashingPrefix;
			
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			//useHasingPrefix가 2의 지수승번으로  검사하면됨 
			int tempBlockNum;
			int howManyHashBlock = calculatePow(useHashingPrefix);
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempBlockNum), sizeof(tempBlockNum));
				if(tempBlockNum == thisBlockNumber) {
					blockPointCount++;
				}
			}
			
			return blockPointCount;
		}
		
		int calculatePow(int twoIndex) {
			int tempTwoIndex = twoIndex;
			int powResult = 1;
			while(tempTwoIndex != 0) {
				powResult *= 2;
				tempTwoIndex --;
			}
			
			return powResult;
		}
		
		void reallocationOnePoint(StudentData currentStudData, int thisBlockNumber) {
			//1개면 hashtable을 두배로 늘린뒤 데이터를 다 들고와서 hashPrefix재설정후(prefix1증가) 
			//block을 하나 더 만들어서(실재로 더 만들 필요는 없음 맨끝4k에 추가 관련 함수 필요) 재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow를 확인후(재귀호출) 현재 데이터 삽입
			int finalBlockNum;
			finalBlockNum = findFinalBlockNum();
			cout << finalBlockNum;
			makeHashTableDouble();
			 
			
			
		}
		
		void makeHashTableDouble() {
			int beforeUseHashingPrefix;
			int afterUseHasingPrefix;
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&beforeUseHashingPrefix), sizeof(beforeUseHashingPrefix));
			
			int howManyHashBlock = calculatePow(beforeUseHashingPrefix);
			hashIO.seekp(4 + howManyHashBlock * 4);
			int tempHashTableNum;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				hashIO.write((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
			}
			
			hashIO.clear();
			afterUseHasingPrefix = beforeUseHashingPrefix + 1;
			hashIO.seekp(0);
			hashIO.write((char*)(&afterUseHasingPrefix), sizeof(afterUseHasingPrefix));
			
			/*hashIO.clear();
			int temptest;
			hashIO.seekg(0);
			hashIO.read((char*)(&temptest), sizeof(temptest));
			cout << temptest << " ";
			cout << hashIO.tellg();*/
			
		}
		
		int findFinalBlockNum() {
			int finalBlockNum = 0;
			int useHashingPrefix;
			
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			int howManyHashBlock = calculatePow(useHashingPrefix);
			
			int tempHashTableNum;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				if(finalBlockNum < tempHashTableNum) {
					finalBlockNum = tempHashTableNum;
				}
			}
			
			return finalBlockNum; 
		}
		
		void reallocationMultiPoint(StudentData currentStudData, int thisBlockNumber) {
			//2개, 4개, 8개 ...면 hashtable을 나눌 필요없이 hashPrefix를 적절히 보고  block을 늘릴 필요없이 데이터를 다 들고와서
			//재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow확인후 (재귀호출 이때는 point가 한개가 될수있음) 현재 데이터 삽입 
		}
		
		//이위로 새로짜는 코드 -----------------------------------------------------------------------------------------------------
		
		
		
		//read all record in studentsData[]
		void readStudentTable() {
			ifstream readStudFile("sampleData.csv");
			
			//input student number by using getline
			string tmp_studentNum;
			getline(readStudFile, tmp_studentNum, '\n');
			studentNum = atoi(tmp_studentNum.c_str());
			studentsData = new StudentData[studentNum];
			
			//input all studentData by using getline
			int dataLocate = 0;
			string tmp_name, tmp_studentID, tmp_score, tmp_advisorID;
			while(getline(readStudFile, tmp_name,',')&&getline(readStudFile, tmp_studentID,',')&&getline(readStudFile, tmp_score,',')&&getline(readStudFile,tmp_advisorID,'\n')) {
				strcpy(studentsData[dataLocate].name,tmp_name.c_str());
				if(tmp_name.length() >= 20) {
					string tmp_re_name = tmp_name.substr(0, 19);
					strcpy(studentsData[dataLocate].name,tmp_re_name.c_str());
				}
				studentsData[dataLocate].studentID = atoi(tmp_studentID.c_str());
				studentsData[dataLocate].score = atof(tmp_score.c_str());
				studentsData[dataLocate].advisorID = atoi(tmp_advisorID.c_str());
				//studentTree.insertItem(studentsData+dataLocate);
				//cout << studentsData[dataLocate].name << "," << studentsData[dataLocate].studentID << "," << studentsData[dataLocate].score << "," << studentsData[dataLocate].advisorID << endl;
				dataLocate ++;
				
			}
			
			readStudFile.close();
			
		}
		
		/*int binToDec(string number) {
			int result = 0, pow = 1;
			for(int i = number.length()-1;i>=0;i--,pow <<= 1) {
				result += (number[i] - '0') * pow;
			}
			return result;
		}*/
		
		//hashFunction key is studentID and hashValue is binary-string type
		string hashFunction(unsigned int studentID) {
			
			if(studentID == 0) {
				return "0";
			} else if (studentID== 1) {
				return "1";
			} else if (studentID % 2 == 0) {
				return hashFunction(studentID / 2) + "0";
			} else {
				return hashFunction(studentID / 2) + "1";
			}
			
		}
		
		//make hashTable and BlockNods
		void calculate_DB_HashTable() {
			HashData tmp_hashData;
			BlockData tmp_blockData;
			
			int howManyInput = 0; // current studentNum                                       
			int useBinaryDigit = 0; // using hashTable's prefix
			int endBlockLocate = 0; // current End block location
			
			hashTablePrefix = useBinaryDigit;
			
			//start input studentData ------------------------------------------------------------------------------1)
			while(howManyInput < studentNum) {
				string hasingValue = hashFunction(studentsData[howManyInput].studentID); // calculate hashingValue by using this data's studentID
				
				//if empty make first hashNode and blockNode --------------------------------------------------------2)
				if(hashNode.empty()) {
					hashNode.push_back(tmp_hashData);
					hashNode[0].pointBlockNum = endBlockLocate;
					tmp_blockData.thisBlockNum = endBlockLocate;
					blockNode.push_back(tmp_blockData);
					endBlockLocate ++;
				}
				
				
				unsigned int thisHashLoc = 0;
				string hashLocBi;
				
				//find hashNode's location mathing this data's hasingValue ------------------------------------------3)
				for(hash_iter = hashNode.begin(); hash_iter != hashNode.end(); hash_iter++) {
					
					//hashLocBi is hashNode's binary value and thisHashLoc is dicimal value
					hashLocBi = hashFunction(thisHashLoc);
					if(hashLocBi.length() < useBinaryDigit) {
						while(hashLocBi.length() != useBinaryDigit) {
							hashLocBi = "0" + hashLocBi;
						}
					}

					bool outThisFor = false;
					
					if(useBinaryDigit == 0) {
						break;
					} else {
						if(hasingValue.substr(hasingValue.length() - useBinaryDigit, useBinaryDigit) == hashLocBi) {
							outThisFor = true;
						}
						
					}
					
					if(outThisFor) {
							break;
					}
					thisHashLoc ++;
				}
				
				//start this data input block ----------------------------------------------------------------4)
				int thisBlockLoc = 0;
					
				//find blockNode's location mathing this data's hasingValue ----------------------------------4-1)
				for(block_iter = blockNode.begin(); block_iter != blockNode.end(); block_iter++) {
					if(hashNode[thisHashLoc].pointBlockNum == blockNode[thisBlockLoc].thisBlockNum) {
						break;
					}
					thisBlockLoc ++;
				}
					
					
				//overflow is exist or not .. input this data in this block -------------------------------------4-2)
				if(blockNode[thisBlockLoc].studentData.size() < 128 ) { // if blockData's size is smaller 128
					blockNode[thisBlockLoc].studentData.push_back(studentsData[howManyInput]);
				
				} else { // if overflow is exist
				
					//count useHashNode is relateing thisBlockNumber, and useHashNode's decimal location store in useHashNode[]
					vector <int> useHashNode;
					int hashPointerNum = 0;
					for(int i = 0; i < hashNode.size(); i++) {
						if(hashNode[i].pointBlockNum == blockNode[thisBlockLoc].thisBlockNum) {
							useHashNode.push_back(i);
						hashPointerNum ++;
						}
					}
					
					BlockData tmp_Allo_blockData;
						
					//according to hash's pointer number, calculate change
					if(hashPointerNum == 1) { // if hash's pointer number = 1, this hashtable is double.
						
						//prefix increase & hashTable double -- same table + same table
						useBinaryDigit ++;
						vector <HashData> tmp_hashNode = hashNode;
						hashNode.insert(hashNode.end(), tmp_hashNode.begin(), tmp_hashNode.end());
						
						//addition new block and this number endBlockLocate
						tmp_Allo_blockData.thisBlockNum = endBlockLocate;
						blockNode.push_back(tmp_Allo_blockData);
						vector <StudentData>::iterator studData_iter;
						
						//calculate "1" +  hashLocBi = new allocation blockNode useing this hashNode's binary value
						string RehashLocBi;
							
						if(useBinaryDigit == 1) {
							RehashLocBi = "1";
						} else {
							RehashLocBi = "1" + hashLocBi;
						}
						
						//start new block's alloction .. using reallocation hashing value
						int alloBloData_loc = 0;
						for(studData_iter = blockNode[thisBlockLoc].studentData.begin(); studData_iter != blockNode[thisBlockLoc].studentData.end();) {
							
							string reHasingValue = hashFunction(blockNode[thisBlockLoc].studentData[alloBloData_loc].studentID);
								
							if(reHasingValue.substr(reHasingValue.length() - useBinaryDigit, useBinaryDigit) == RehashLocBi) {
								blockNode[endBlockLocate].studentData.push_back(blockNode[thisBlockLoc].studentData[alloBloData_loc]);

								studData_iter = blockNode[thisBlockLoc].studentData.erase(studData_iter);
							} else {
								studData_iter ++;
								alloBloData_loc++;
							}
								
																	
						}
						
						//this current data in block
						if(hasingValue.substr(hasingValue.length() - useBinaryDigit, useBinaryDigit) == RehashLocBi) {
							blockNode[endBlockLocate].studentData.push_back(studentsData[howManyInput]);
						} else {
							blockNode[thisBlockLoc].studentData.push_back(studentsData[howManyInput]);
						}
							
						// hashpointer realloction
						int newHashLoc = 1;
						for(int i = 1; i < useBinaryDigit; i ++) {
							newHashLoc *= 2;
						}
						
						hashNode[thisHashLoc + newHashLoc].pointBlockNum = endBlockLocate;
						
						endBlockLocate ++;
						
					} else { // if hash's pointer number is not 1, this hashTable use.
							//and hash's pointer number is 2, 4, 8, ....
							
						//addition new block and this number endBlockLocate
						tmp_Allo_blockData.thisBlockNum = endBlockLocate;
						blockNode.push_back(tmp_Allo_blockData);
						
						//according do pointer Number, useBinaryDIgit diffrent! use n digit : pointer 2 -> same, pointer 4 -> n - 1....
						
						int useHashBi_loc = 0;
						int tmp_pointerNum = hashPointerNum;
						while(tmp_pointerNum != 1) {
							tmp_pointerNum /= 2;
							useHashBi_loc ++;
						}
						
						//I using first hashNode's value
						string first_level_RehashLocBi = hashFunction(useHashNode[0]);
						
						if(first_level_RehashLocBi.length() < useBinaryDigit) {
							while(first_level_RehashLocBi.length() != useBinaryDigit) {
								first_level_RehashLocBi = "0" + first_level_RehashLocBi;
							}
						}
							
						int manyPointDigit = useBinaryDigit - useHashBi_loc + 1; // this is useBinaryDigit in this hashPointers
							
						string RehashLocBi = first_level_RehashLocBi.substr(first_level_RehashLocBi.length() - (manyPointDigit - 1), (manyPointDigit - 1));
						RehashLocBi = "1" + RehashLocBi; // this is reallocate hashingValue : before Block -> new Block
						
						//start new block's alloction .. using reallocation hashing value
						vector <StudentData>::iterator studData_iter;
						int alloBloData_loc = 0;
						for(studData_iter = blockNode[thisBlockLoc].studentData.begin(); studData_iter != blockNode[thisBlockLoc].studentData.end();) {
								
							string reHasingValue = hashFunction(blockNode[thisBlockLoc].studentData[alloBloData_loc].studentID);
							
							if(reHasingValue.substr(reHasingValue.length() - manyPointDigit, manyPointDigit) == RehashLocBi) {
								blockNode[endBlockLocate].studentData.push_back(blockNode[thisBlockLoc].studentData[alloBloData_loc]);

								studData_iter = blockNode[thisBlockLoc].studentData.erase(studData_iter);
							} else {
								studData_iter ++;
								alloBloData_loc++;
							}
																
						}
							
						//this current data in block
						if(hasingValue.substr(hasingValue.length() - useBinaryDigit, useBinaryDigit) == RehashLocBi) {
							blockNode[endBlockLocate].studentData.push_back(studentsData[howManyInput]);
						} else {
							blockNode[thisBlockLoc].studentData.push_back(studentsData[howManyInput]);						
						}
						
						// hashpointer realloction
						int changeHashPointNum = hashPointerNum / 2;
						for(int k = 0; k < (hashPointerNum/2); k++) {
							hashNode[useHashNode[k*2 + 1]].pointBlockNum = endBlockLocate;
							changeHashPointNum ++;
						}
							
						endBlockLocate ++;

				 	}
										
				}
			
				howManyInput ++;
				hashTablePrefix = useBinaryDigit;
			}
		}
		
		//make StudentDB
		void writeStudentDB() {
			
			// seekp, g사용해서 student.DB에 넣어야함
			ofstream fo;
			fo.open("Student.DB", ios::out | ostream::binary);
			int i = 0;
			vector <StudentData>::iterator studData_iter;
			for(block_iter = blockNode.begin(); block_iter != blockNode.end(); block_iter++) {
				int j = 0;
				fo.seekp(4096 * i);
				for(studData_iter = blockNode[i].studentData.begin(); studData_iter != blockNode[i].studentData.end(); studData_iter++) {
					// file in blockNode's studentData
					fo.write((char*)(&(blockNode[i].studentData[j].name)), sizeof(blockNode[i].studentData[j].name));
					//fo.seekp(4096 * i + '20 * j);
					fo.write((char*)(&(blockNode[i].studentData[j].studentID)), sizeof(blockNode[i].studentData[j].studentID));
					fo.write((char*)(&(blockNode[i].studentData[j].score)), sizeof(blockNode[i].studentData[j].score));
					fo.write((char*)(&(blockNode[i].studentData[j].advisorID)), sizeof(blockNode[i].studentData[j].advisorID));
					
					//if(j==0)break;
				
					/*fo << blockNode[i].studentData[j].name;
					fo << blockNode[i].studentData[j].studentID;
					fo << blockNode[i].studentData[j].score;
					fo << blockNode[i].studentData[j].advisorID;*/
					
					j++;
				}
				//if(i == 0) break;
				i ++;
			}
			
			fo.close();		
			
			ifstream fi;
			fi.open("Student.DB", ios::in | ostream::binary);
			char a[20];
			unsigned int b;
			float c;
			unsigned int d;
			
			fi.seekg(4096);
			
			for(int q = 0; q < 4096; q += 32) {
				fi.read((char*)(&a), sizeof(a));
				if(a[0] == NULL) {
					break;
				}
				fi.read((char*)(&b), sizeof(b));
				fi.read((char*)(&c), sizeof(c));
				fi.read((char*)(&d), sizeof(d));
				for(int w = 0; w < 20; w ++) {
					cout << a[w];
				}
				cout << endl;
				cout <<b << endl;
				cout <<c << endl;
				cout << d <<endl;
				cout << endl;
			}
			
			//fi.seekg(4096 * i);
			
			
			
			fi.close();
			
		}
		
		//make HashTable
		void writeHashTable() {
			ofstream fo;
			fo.open("Students.hash", ios::out | ostream::binary);
			
			int i = 0;
			//fo << hashTablePrefix; // first in prefix
			fo.write((char*)(&hashTablePrefix), sizeof(int));
			for(hash_iter = hashNode.begin(); hash_iter != hashNode.end(); hash_iter++) {
				fo.write((char*)(&(hashNode[i].pointBlockNum)), sizeof(int));
				//fo << hashNode[i].pointBlockNum; // file in hashNode's blockNumber
				i++;
			}
			
			fo.close();
		}
		
		//print hashTable .. prefix and hashpointer's blockNode
		void hashTablePrint() {
			ifstream fi;
			fi.open("Students.hash", ios::in | ostream::binary);
			int i;
			cout << "-----------this hashTable print start-------------" << endl;
			fi.read((char*)(&i), sizeof(int));
			cout << "prefix : " << i << endl;
			/*for(hash_iter = hashNode.begin(); hash_iter != hashNode.end(); hash_iter++) {
				fi.read(reinterpret_cast<char*>(&i), sizeof(int));
				cout << i << endl;
			}*/
			while(fi.read((char*)(&i), sizeof(int)))
			{
				//fi.read(reinterpret_cast<char*>(&i), sizeof(int));
				cout << i << endl;
			}
			
			cout << "------------this hashTable print end--------------" << endl;		
			
			fi.close();
		}
		
		void make_B_plusTree() {
			int k = 0;
			nodeData = new nData[studentNum];
			for(int i = 0; i < blockNode.size(); i++){
				for(int j=0; j < blockNode[i].studentData.size();j++){
					nodeData[k].score = blockNode[i].studentData[j].score;
					nodeData[k].bNum = blockNode[i].thisBlockNum;
					studentTree.insertItem(nodeData+k);
					k++;
				}
			}
		}
		
		void kthNodePrint() {
			int k;
			cout << "what is k? : ";
			cin >> k;
			if(k > 0){
				studentTree.kSearch(k-1);
			}
			else{
				cout << "invalid input!" << endl;
				kthNodePrint();
			}
		}
		
		void idxOut(){
  			ofstream fo;
			nData* leafNodes;
			char comma = ',';
			char enter = '\n';
			leafNodes = studentTree.getLeaves(studentNum);
 			fo.open("Student_score.idx", ostream::binary);  
  			if (!fo) {   // if(fo.fail())
    			cerr << "idx open failed.." << endl;
    			exit(1);
  			}

  			for (int i = 0; i < studentNum; i++){ //save leaf nodes
   	 			//fo.write((char*)&i, sizeof(int));
  				fo.write((char*)&(leafNodes[i].score),sizeof(float));
				fo.write((char*)&comma,sizeof(char));
				fo.write((char*)&(leafNodes[i].bNum),sizeof(int));
				fo.write((char*)&enter,sizeof(char));			
			}
			fo.close();

  
		}
	
	
	
};

bool Bptree::insertItem(nData* k)
{
	
	node* trace[TR];	// node stack of trace to insert location
	int dir[TR];		// index stack of trace to insert location
	float Key;
	int i=0;				//index of stack			

	node* Right, *p;	//Right: insert node
						//p: point node that we check
	Right = (node*)k;
	p = root;
	

			
	if (root == NULL)	//first
	{
		root = new node();
		root->branch[0] = NULL;	//in leafnode: next leaf node
								//in internal node: next level node
		root->Key[0] = k->score;
		root->branch[1] = (node*)k;//next level node
		root->count = M + 1;
		return true;
	}

	while (true)	// go to leaf node
	{
		int j;
		trace[i] = p;
		for (j=0; j<p->count%M; j++)	//count is not always less than M
			if (p->Key[j] >= k->score)
			{
				dir[i] = j;
				break;
			}
		if (j == p->count%M)
			dir[i] = p->count%M;
		if (p->count/M == 1)
			break;
		p = p->branch[j];
		i++;
	}						// 이 루프에서 나오면 p는 Key값이 삽일될 노드. 

	
	// start insert 
	Key = k->score;
	while (i != -1)
	{
		int path = dir[i];
		p = trace[i];
		if (p->count%M != M-1)	// no overflow
		{
			for (int m=p->count%M; m>path; m--)	// make space 
			{
				p->Key[m] = p->Key[m-1];
				p->branch[m+1] = p->branch[m];
			}
			p->Key[path] = Key;		
			p->branch[path+1] = Right;	
			p->count++;
			break;
		}
	
		else	//overflow
		{
			float nodeKey[M];
			node* nodeBranch[M+1];
			node* newNode;
			int j, j2;
			newNode = (node*)malloc(sizeof(node));
			
			nodeBranch[0] = p->branch[0];
			for (j=0, j2=0; j<M; j++, j2++)		// save value temporary
			{
				if (j == path)
				{
					nodeKey[j] = Key;
					nodeBranch[j+1] = Right;
					j++;
					if (j>=M) 
						break;
				}
				nodeKey[j] = p->Key[j2];
				nodeBranch[j+1] = p->branch[j2+1];
			}
			for (j=0; j<M/2; j++)  //front of middle key
			{
				p->Key[j] = nodeKey[j];
				p->branch[j+1] = nodeBranch[j+1];
			}
			newNode->branch[0] = nodeBranch[M/2+1];
			for (j=0; j<M/2; j++)	//rear of middle key 
			{
				newNode->Key[j] = nodeKey[M/2+1+j];
				newNode->branch[j+1] = nodeBranch[M/2+2+j];
			}

			if (p->count/M == 1) // leaf node
			{
				newNode->branch[0] = p->branch[0];	// link list.
				p->branch[0] = newNode;				//new node is on the right of p
				p->Key[M/2] = nodeKey[M/2];			// leaf node also has that upper level value 
				p->branch[M/2+1] = nodeBranch[M/2+1];
				p->count = M + M/2 + 1;
				newNode->count = M + M/2;
			}
			else
			{
				p->count = M/2;
				newNode->count = M/2;
				p->branch[0] = nodeBranch[0];
			}

			Key = nodeKey[M/2];	// send to upper level 
			Right = newNode;	// right child node 
		}
		i--;//stack index descreate like stack pop
	}
	if (i == -1)	// root overflow
	{
		root = (node*)malloc(sizeof(node));
		root->count = 1;
		root->branch[0] = trace[0];
		root->branch[1] = Right;
		root->Key[0] = Key;
	}

	return true;
}



void Bptree::kSearch(int k){
	node* p = root;
	int nodeNum = 0;


	if (p != NULL)
	{	
		while (true)	// go to leaf node
		{
			if (p->count/M == 1)
				break;
			p = p->branch[0];
		}
		cout << "(score,block number)" << endl;				
		while (p != NULL)
		{
			for (int j=0; j<p->count%M; j++)
			{
				if(nodeNum == k){
					cout << "(" << ((nData*)p->branch[j+1])->score << " , " << ((nData*)p->branch[j+1])-> bNum << ")" << endl;
					
				}
			}
			nodeNum++;
			if(nodeNum == k+1)
				break;
			p = p->branch[0];
		}
		if(p == NULL){
			cout << "range over!" << endl;
		}
	}
}

nData* Bptree::getLeaves(int n){
	nData* leaves;
	leaves = new nData[n];
	int i = 0;
	node* p = root;
	if (p != NULL)
	{	
		while (true)	// go to leaf node
		{
			if (p->count/M == 1)
				break;
			p = p->branch[0];
		}				
		while (p != NULL)
		{
			for (int j=0; j<p->count%M; j++)
			{
				{
					leaves[i].score = ((nData*)p->branch[j+1])->score;
					leaves[i].bNum=((nData*)p->branch[j+1])-> bNum;
					i++;
				}
			}
			p = p->branch[0];
		}
		
	}
	return leaves;
}


int main(int argc, char** argv) {
	StudentsFileStruct studentsFS;
	studentsFS.fileHashAndDBOpen();
	studentsFS.readStudTableAndUpdateFile();
	/*studentsFS.readStudentTable();
	studentsFS.calculate_DB_HashTable();
	studentsFS.writeStudentDB();
	studentsFS.writeHashTable();
	studentsFS.hashTablePrint();
	studentsFS.make_B_plusTree();
	studentsFS.kthNodePrint();
	studentsFS.idxOut();*/
	return 0;
}
