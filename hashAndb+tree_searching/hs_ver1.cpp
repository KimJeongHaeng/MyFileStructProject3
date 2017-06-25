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

typedef struct InputProData {
	char name[20];
	unsigned int profID;
	int salary;
}ProfessorData;

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

class StuAndProFileStruct {
	private:
		int studentNum;
		fstream hashIO;
		fstream DBIO;
		vector <StudentData> tempStudentDatas;
		
		int professorNum;
		fstream hashIOPro;
		fstream DBIOPro;
		vector <ProfessorData> tempProfessorDatas;
		
		//이위로 새로짜는 코드 
		
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
			
		}
		
		void readStudTableAndUpdateFile() { //자료들을 한줄씩 읽음 
			ifstream readStudFile("sampleData.csv");
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
			hashTablePrint();
			
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
				DBIO.clear();
				DBIO.seekg((thisBlockNumber * 4096) + i);
				DBIO.read((char*)(&isNull), sizeof(isNull));
				
				
				if(isNull == NULL || DBIO.tellg() == -1) {
					inputLocation = i;
					//cout << i << " ";
					break;
				}
			}
			
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
				//cout << "onepointover" <<endl; 
			} else {
				//2개, 4개, 8개 ...면 hashtable을 나눌 필요없이 hashPrefix를 적절히 보고  block을 늘릴 필요없이 데이터를 다 들고와서
				//재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
				//다시 넣을 곳의 block의 overflow확인후 (재귀호출 이때는 point가 한개가 될수있음) 현재 데이터 삽입 
				reallocationMultiPoint(currentStudData, thisBlockNumber);
				//cout << blockPointCount << "multipoint over" << endl;
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
			//cout << finalBlockNum;
			makeHashTableDouble();
			
			reallocateBlocks(currentStudData, thisBlockNumber, finalBlockNum);
			reallocateHashTable(currentStudData, thisBlockNumber, finalBlockNum);
			
			
		}
		
		void makeHashTableDouble() {
			int beforeUseHashingPrefix;
			int afterUseHasingPrefix;
			vector <int> tempHashTable;
			
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&beforeUseHashingPrefix), sizeof(beforeUseHashingPrefix));
			
			int howManyHashBlock = calculatePow(beforeUseHashingPrefix);
			
			int tempHashTableNum;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				tempHashTable.push_back(tempHashTableNum);
			}
			
			hashIO.clear();
			hashIO.seekp(4 + howManyHashBlock * 4);
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.write((char*)(&tempHashTable[i]), sizeof(tempHashTable[i]));
			}
			
			hashIO.clear();
			afterUseHasingPrefix = beforeUseHashingPrefix + 1;
			hashIO.seekp(0);
			hashIO.write((char*)(&afterUseHasingPrefix), sizeof(afterUseHasingPrefix));
			//testPrint();
			//cout << "endline \n";
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
		
		void reallocateBlocks(StudentData currentStudData, int thisBlockNumber, int finalBlockNum) {
			StudentData tempOneStudData;
			//StudentData emptyStudData;
			char isNull;
			char tempNullChar[20] = {NULL};
			unsigned int tempNullInt = NULL;
			float tempNullFloat = NULL;
			
			for(int i = 0; i < 4096; i += sizeof(currentStudData)) {
			//for(int i = 0; i < 4096; i ++) {
				DBIO.clear();
				DBIO.seekg((thisBlockNumber * 4096) + i);
				DBIO.read((char*)(&isNull), sizeof(isNull));
				
				if(isNull == NULL || DBIO.tellg() == -1) {
					break;
				}
				
				DBIO.clear();
				DBIO.seekg((thisBlockNumber * 4096) + i);
				DBIO.read((char*)(&tempOneStudData.name), sizeof(tempOneStudData.name));
				DBIO.read((char*)(&tempOneStudData.studentID), sizeof(tempOneStudData.studentID));
				DBIO.read((char*)(&tempOneStudData.score), sizeof(tempOneStudData.score));
				DBIO.read((char*)(&tempOneStudData.advisorID), sizeof(tempOneStudData.advisorID));
				tempStudentDatas.push_back(tempOneStudData);
				
				DBIO.clear();
				DBIO.seekp(thisBlockNumber * 4096 + i);
				//cout << currentStudData.name << " ";
				DBIO.write((char*)(&tempNullChar), sizeof(tempNullChar));
				DBIO.write((char*)(&tempNullInt), sizeof(tempNullInt));
				DBIO.write((char*)(&tempNullFloat), sizeof(tempNullFloat));
				DBIO.write((char*)(&tempNullInt), sizeof(tempNullInt));
				
			}			
			
			int useHashingPrefix;
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			int beforeHashValue = findHashValue(currentStudData.studentID, useHashingPrefix - 1);
			
			for(int i = 0; i < tempStudentDatas.size(); i ++) {
				int newHashValue = findHashValue(tempStudentDatas[i].studentID, useHashingPrefix);
				if(newHashValue == beforeHashValue) {
					inputData(tempStudentDatas[i], thisBlockNumber);
				} else {
					inputData(tempStudentDatas[i], finalBlockNum + 1);
				}
			}
			
			tempStudentDatas.clear();
			
			int thisHashValue = findHashValue(currentStudData.studentID, useHashingPrefix);
			if(thisHashValue == beforeHashValue) {
				inputData(currentStudData, thisBlockNumber);
			} else {
				inputData(currentStudData, finalBlockNum + 1);
			}
				
		}
		
		void reallocateHashTable(StudentData currentStudData, int thisBlockNumber, int finalBlockNum) {
			int useHashingPrefix;
			int increaseFinalBlockNum = finalBlockNum + 1;
			vector <int> tempChangeHashTable;
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			int howManyHashBlock = calculatePow(useHashingPrefix);
			
			int tempHashTableNum;
			int changeHashPointNum = 0;
			int count = 0;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				if(thisBlockNumber == tempHashTableNum) {
					count++;
					if(changeHashPointNum == 0) {
						changeHashPointNum = 1;
					} else {
						tempChangeHashTable.push_back(i);
						changeHashPointNum = 0;
					}
					
				}
			}
			//cout << "현재 pointer갯수" << count << endl;
			for(int i = 0; i < tempChangeHashTable.size(); i++) {
				hashIO.clear();
				hashIO.seekp(4 + tempChangeHashTable[i] * 4);
				hashIO.write((char*)(&increaseFinalBlockNum), sizeof(increaseFinalBlockNum));
			}
			
			//testPrint();
			//cout << "test \n";
			
			
		}
		
		void reallocationMultiPoint(StudentData currentStudData, int thisBlockNumber) {
			//2개, 4개, 8개 ...면 hashtable을 나눌 필요없이 hashPrefix를 적절히 보고  block을 늘릴 필요없이 데이터를 다 들고와서
			//재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow확인후 (재귀호출 이때는 point가 한개가 될수있음) 현재 데이터 삽입
			
			int finalBlockNum;
			finalBlockNum = findFinalBlockNum();
			//cout << finalBlockNum;
			
			
			reallocateBlocksMulti(currentStudData, thisBlockNumber, finalBlockNum);
			reallocateHashTable(currentStudData, thisBlockNumber, finalBlockNum);
			
			
		}
		
		void reallocateBlocksMulti(StudentData currentStudData, int thisBlockNumber, int finalBlockNum) {
			StudentData tempOneStudData;
			//StudentData emptyStudData;
			char isNull;
			char tempNullChar[20] = {NULL};
			unsigned int tempNullInt = NULL;
			float tempNullFloat = NULL;
			
			for(int i = 0; i < 4096; i += sizeof(currentStudData)) {
			//for(int i = 0; i < 4096; i ++) {
				DBIO.clear();
				DBIO.seekg((thisBlockNumber * 4096) + i);
				DBIO.read((char*)(&isNull), sizeof(isNull));
				//cout << isNull << "구분자";
				
				if(isNull == NULL || DBIO.tellg() == -1) {
					break;
				}
				
				DBIO.clear();
				DBIO.seekg((thisBlockNumber * 4096) + i);
				DBIO.read((char*)(&tempOneStudData.name), sizeof(tempOneStudData.name));
				DBIO.read((char*)(&tempOneStudData.studentID), sizeof(tempOneStudData.studentID));
				DBIO.read((char*)(&tempOneStudData.score), sizeof(tempOneStudData.score));
				DBIO.read((char*)(&tempOneStudData.advisorID), sizeof(tempOneStudData.advisorID));
				tempStudentDatas.push_back(tempOneStudData);
				
				DBIO.clear();
				DBIO.seekp(thisBlockNumber * 4096 + i);
				//cout << currentStudData.name << " ";
				DBIO.write((char*)(&tempNullChar), sizeof(tempNullChar));
				DBIO.write((char*)(&tempNullInt), sizeof(tempNullInt));
				DBIO.write((char*)(&tempNullFloat), sizeof(tempNullFloat));
				DBIO.write((char*)(&tempNullInt), sizeof(tempNullInt));
				
			}
			
			/*cout << tempStudentDatas[127].name;
			cout << tempStudentDatas[127].studentID;
			cout << tempStudentDatas[127].score;
			cout << tempStudentDatas[127].advisorID;*/
			
			
			int useHashingPrefix;
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			int blockPointCount = findBlockPointCount(thisBlockNumber);
			
			int forRealusedPrefix = -1; //포인터의 개수가 1개면 안빼고 2개면 1빼고 4개면 2빼고 ... 
			while(blockPointCount != 1) {
				forRealusedPrefix ++;
				blockPointCount = blockPointCount/2;
				
			}
			
			int beforeHashValue = findHashValue(currentStudData.studentID, useHashingPrefix - 1 - forRealusedPrefix);
			
			for(int i = 0; i < tempStudentDatas.size(); i ++) {
				int newHashValue = findHashValue(tempStudentDatas[i].studentID, useHashingPrefix - forRealusedPrefix);
				if(newHashValue == beforeHashValue) {
					inputData(tempStudentDatas[i], thisBlockNumber);
				} else {
					inputData(tempStudentDatas[i], finalBlockNum + 1);
				}
			}
			
			tempStudentDatas.clear();
			
			int thisHashValue = findHashValue(currentStudData.studentID, useHashingPrefix - forRealusedPrefix);
			if(thisHashValue == beforeHashValue) {
				inputData(currentStudData, thisBlockNumber);
			} else {
				inputData(currentStudData, finalBlockNum + 1);
			}
				
		}
		
		
		
		void hashTablePrint() {
			int useHashingPrefix;
			
			hashIO.clear();
			hashIO.seekg(0);
			hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			int howManyHashBlock = calculatePow(useHashingPrefix);
			cout << "-----hashtable-----" << endl;
			cout << "hashTablePrefix :" << useHashingPrefix << endl;
			int tempHashTableNum;
		
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIO.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				cout << tempHashTableNum << endl;
			}
			
			cout << "-----hashtableEnd-----" << endl;
			
			/*int count = 0;
			for(int i = 0; i < 4096; i += 32) {
				char isNull;
				DBIO.clear();
				DBIO.seekg(i + 4096 * 31);
				DBIO.read((char*)(&isNull), sizeof(isNull));
				//cout << isNull << "구분자";
				
				if(isNull == NULL || DBIO.tellg() == -1) {
					break;
				}
				StudentData tempOneStudData;
				cout << count << " ";
				count ++;
				
				DBIO.clear();
				DBIO.seekg(i);
				//DBIO.read((char*)(&tempOneStudData.name), sizeof(tempOneStudData.name));
				//DBIO.read((char*)(&tempOneStudData.studentID), sizeof(tempOneStudData.studentID));
				//DBIO.read((char*)(&tempOneStudData.score), sizeof(tempOneStudData.score));
				//DBIO.read((char*)(&tempOneStudData.advisorID), sizeof(tempOneStudData.advisorID));
				
				int useHashingPrefix;
				
				hashIO.clear();
				hashIO.seekg(0);
				hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
				
				//cout << findHashValue(tempOneStudData.studentID, useHashingPrefix) << " ";
				//cout << tempOneStudData.name << " ";
				//cout <<tempOneStudData.studentID << " ";
				//cout << tempOneStudData.score << " ";
				//cout << tempOneStudData.advisorID << "\n";
			}*/
		}
		
		//이위로 새로짜는 코드 -----------------------------------------------------------------------------------------------------
		
		/*void make_B_plusTree() {
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

  
		}*/
	
	
	
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
	StuAndProFileStruct stuAndProFS;
	stuAndProFS.fileHashAndDBOpen();
	stuAndProFS.readStudTableAndUpdateFile();
	/*studentsFS.make_B_plusTree();
	studentsFS.kthNodePrint();
	studentsFS.idxOut();*/
	return 0;
}
