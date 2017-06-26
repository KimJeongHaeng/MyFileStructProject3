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
#define M 3  // 홀수 차수 생성 
#define MAX 500	// 큐의 크기
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

typedef struct Node
{
	int count;		// 노드에 저장된 Key의 수, 인덱스 노드와 리프노드 구별 플래그	
	float Key[M-1][2];			 // Key 
	struct Node* branch[M];  // 주소 
} node;

class BplusTree{
	private:
		node* root;		// root를 가리키는 노드
		vector<int> blockBound;
		ifstream fi;
		int dataCount;
		
	public:
		BplusTree(){
			root = NULL;
			dataCount = 0;
		}
		void makeStudentB();
		int* search(float k);		// Key를 탐색하는 함수 
		void rangeSearch(float k,float l);	// 순차탐색(리프노드) 출력  함수 
		int* insertItem(float k);		// Key 삽입 함수

};

void BplusTree::makeStudentB(){
	fi.open("Student.DB", ifstream::binary); 
  	if (!fi) { 
    	cerr << "idx open failed.." << endl;
    	exit(1);
  	}
  	float b;
  	
  	int i = 0;
  	int j = 0;
  	int k = 0;
  	while(!fi.eof()){
  		fi.seekg((j*4096)+24 + (i * 32));
  		fi.read((char*)&b,sizeof(float));
  		if(b == 0){
  			j++;
  			i = 0;
  			blockBound.push_back(k-1);
  			//cout << (j*4096)+24 + (i * 32) << endl;
		}
		else{
			insertItem(b);
			//cout << (j*4096)+24 + (i * 32) << endl;
			k++;
			if((j*4096)+24 + (i * 32)+32 >= 4096*(j+1)){
				j++;
				blockBound.push_back(k-1);
				i = 0;
			}
			else{
				i++;
			}
		}	
	}
	fi.clear();
}



int* BplusTree::search(float k)
{
	node* p = root;
	int path;
	if (p == NULL)
		return NULL;
	while (1)	// p가 leaf노드일때까지 탐색
	{	
		int j;
		for (j=0; j<p->count%M; j++)	// 한 노드에서 경로를 결정
		{
			if (p->Key[j][0] >= k)
			{
				path = j;
				break;
			}
		}
		if (j == p->count%M)
			path = p->count%M;

		if (p->count/M == 1)
				break;
	
		p = p->branch[j];
		
	}				
	if (p->Key[path][0] == k && p->count%M != path)
	{
		return (int*)p->branch[path+1];
	}
	else 
		return NULL;
}

void BplusTree::rangeSearch(float k,float l)
{
	ofstream range_query;
	range_query.open("query.res", ios::out | ios::binary);
	
	int path, j;//from
	node* p = root;
	
	int path_2, j_2;//to
	node* p_2 = root;

	if (p != NULL)
	{	
		while (true)	// p가 leaf노드 일때까지 탐색
		{
			//int j;
			for (j=0; j<p->count%M; j++)	// 한 노드에서 경로를 결정
			{	
				if (p->Key[j][0] >= k)
				{
					path = j;
					break;
				}
			}
			if (j == p->count%M)
				path = p->count%M;
			if (p->count/M == 1)
				break;
			p = p->branch[j];
		}
		if (p->Key[path][0] == k || k == 0)	// k가 0이면 minimum부터 다 출력한다. 
		{
			bool end = false;
			int* s = search(l+0.1);
			while (p != NULL)
			{
				for (j=0; j<p->count%M; j++)
				{
					int bc = *(int*)p->branch[j+1];
					
					if(bc == *s){
					//	cout << bc << " " << bc_2 << endl;
						end = true;
						break;
					}
					int bb;
					char name[20];
					int id;
					float score;
					int aId;
					for(bb = 0; bb < blockBound.size(); bb++){
						if(bc <= blockBound[bb]){
							break;
						}
					}
					fi.clear();
					if(bb == 0){
						fi.seekg(bb*4096 + (bc)*32,ios::beg);
						fi.read((char*)(&name),sizeof(name));
						fi.read((char*)&id,sizeof(int));
						fi.read((char*)&score,sizeof(float));
						fi.read((char*)&aId,sizeof(int));
						range_query.write((char*)(&name), sizeof(name));
						range_query.write((char*)(&id), sizeof(id));
						range_query.write((char*)(&score), sizeof(score));
						range_query.write((char*)(&aId), sizeof(aId));
						cout << name << " " << id << " " << score << " " << aId << endl;
					}
					else{
						fi.seekg(bb*4096 + (bc-blockBound[bb-1]-1)*32,ios::beg);
						fi.read((char*)(&name),sizeof(name));
						fi.read((char*)&id,sizeof(int));
						fi.read((char*)&score,sizeof(float));
						fi.read((char*)&aId,sizeof(int));
						range_query.write((char*)(&name), sizeof(name));
						range_query.write((char*)(&id), sizeof(id));
						range_query.write((char*)(&score), sizeof(score));
						range_query.write((char*)(&aId), sizeof(aId));
						cout << name << " " << id << " " << score << " " << aId << endl;
						//cout << name << endl; 
					}
					//fi.read((char*)&c,sizeof(int));
					//cout << c << " \n ";
				}
				if(end){
					break;
				}
				p = p->branch[0];
			}
		}
	}
	range_query.close();
}



int* BplusTree::insertItem(float k)
{
	int loc = dataCount;
	node* trace[TR];	// 삽입될 경로를 저장할 스택용도의 배열
	int dir[TR];
	float Key;
	int  i;				

	node* upRight, *p;
	int* insertFileLocation = new int;
	*insertFileLocation = loc;
	upRight = (node*)insertFileLocation;
	i = 0;	// trace[]의 index

	p = root;	// p를 가지고 삽입될 위치를 탐색

			
	if (root == NULL)
	{
		root = new node();
		root->branch[0] = NULL;
		root->Key[0][0] = k;
		root->Key[0][1] = loc;
		root->branch[1] = (node*)insertFileLocation;
		root->count = M + 1;
		dataCount++;
		return insertFileLocation;
	}

	while (1)	// p가 leaf노드 일때까지 탐색
	{
		int j;
		trace[i] = p;
		for (j=0; j<p->count%M; j++)	// 한 노드에서 경로를 결정
			if (p->Key[j][0] >= k)
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

	
	// 이제 본격적인 삽입을 시작. 
	Key = k;
	while (i != -1)
	{
		int path = dir[i];
		p = trace[i];
		if (p->count%M != M-1)	// 삽입해도 overflow가 생기지 않으면
		{
			int m;
			for (m=p->count%M; m>path; m--)	// 삽입될 칸부터 끝까지 한칸씩 뒤로. 
			{
				p->Key[m][0] = p->Key[m-1][0];
				p->Key[m][1] = p->Key[m-1][1];
				p->branch[m+1] = p->branch[m];
			}
			p->Key[path][0] = Key;		// Key값을 삽입
			p->Key[path][1] = loc;
			p->branch[path+1] = upRight;	// branch를 관리. 
			p->count++;
			break;
		}
	
		else	// 삽입하면 overflow가 생기는 경우
		{
			float nodeKey[M][2];
			node* nodeBranch[M+1];
			node* newNode;
			int j, j2;
			newNode = new node();
			
			nodeBranch[0] = p->branch[0];
			for (j=0, j2=0; j<M; j++, j2++)		// 임시로 크기 M+1인 노드에 순서대로 복사. 
			{
				if (j == path)
				{
					nodeKey[j][0] = Key;
					nodeKey[j][1] = loc;
					nodeBranch[j+1] = upRight;
					j++;
					if (j>=M) 
						break;
				}
				nodeKey[j][0] = p->Key[j2][0];
				nodeKey[j][1] = p->Key[j2][1];
				nodeBranch[j+1] = p->branch[j2+1];
			}
			for (j=0; j<M/2; j++)
			{
				p->Key[j][0] = nodeKey[j][0];
				p->Key[j][1] = nodeKey[j][1];
				p->branch[j+1] = nodeBranch[j+1];
			}
			newNode->branch[0] = nodeBranch[M/2+1];
			for (j=0; j<M/2; j++)	// 가운데 Key 다음부터는 새로생긴 노드에 복사한다. 
			{
				newNode->Key[j][0] = nodeKey[M/2+1+j][0];
				newNode->Key[j][1] = nodeKey[M/2+1+j][1];
				newNode->branch[j+1] = nodeBranch[M/2+2+j];
			}

			// 만약에 p가 리프노드이면 약간의 수정
			if (p->count/M == 1)
			{
				newNode->branch[0] = p->branch[0];	// sequencial pointer 관리
				p->branch[0] = newNode;
				p->Key[M/2][0] = nodeKey[M/2][0];		// 올릴 Key값을 리프노드에도 남김. 
				p->Key[M/2][1] = nodeKey[M/2][1];
				p->branch[M/2+1] = nodeBranch[M/2+1];
				p->count = M + M/2 + 1;
				newNode->count = M + M/2;
			}
			else
			{
				p->count = newNode->count = M/2;
				p->branch[0] = nodeBranch[0];
			}

			Key = nodeKey[M/2][0];	// 가운데 Key를 올리는 Key로 한다. 
			loc = nodeKey[M/2][1];
			upRight = newNode;	// 새로 만든 node를 올리는 값의 오른쪽 자식으로 
		}
		i--;
	}
	if (i == -1)	// root에서 overflow가 생겼을 경우
	{
		root = new node();
		root->count = 1;
		root->branch[0] = trace[0];
		root->branch[1] = upRight;
		root->Key[0][0] = Key;
		root->Key[0][1] = loc;
	}

	dataCount++;
	return insertFileLocation;
}

BplusTree *bpt;

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
		
		
		
	public:
		void fileHashAndDBOpen() {
			
			ofstream hashIO_temp;
			ofstream DBIO_temp;
			
			hashIO_temp.open("Students.hash", ios::out | ios::binary);
			hashIO_temp.close();
				
			hashIO.open("Students.hash", ios::in | ios::out | ios::binary);
			
			hashIO.seekp(0);
			int firstHashTablePrefix = 0;
			int hashPointBlockNum = 0;
			hashIO.write((char*)(&firstHashTablePrefix), sizeof(firstHashTablePrefix));
			hashIO.write((char*)(&hashPointBlockNum), sizeof(hashPointBlockNum));
			
			DBIO_temp.open("Student.DB", ios::out | ios::binary);
			DBIO_temp.close();
			
			DBIO.open("Student.DB", ios::in | ios::out | ios::binary);
			
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
					
					break;
				}
			}
			
			DBIO.clear();
			DBIO.seekp(thisBlockNumber * 4096 + inputLocation);
			
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
			
			reallocateBlocksMulti(currentStudData, thisBlockNumber, finalBlockNum);
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
			cout << "-----Student hashtable-----" << endl;
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
				DBIO.seekg(i + 4096 * 1);
				DBIO.read((char*)(&isNull), sizeof(isNull));
				//cout << isNull << "구분자";
				
				if(isNull == NULL || DBIO.tellg() == -1) {
					break;
				}
				StudentData tempOneStudData;
				cout << count << " ";
				count ++;
				
				DBIO.clear();
				DBIO.seekg(i + 4096 * 1);
				DBIO.read((char*)(&tempOneStudData.name), sizeof(tempOneStudData.name));
				DBIO.read((char*)(&tempOneStudData.studentID), sizeof(tempOneStudData.studentID));
				DBIO.read((char*)(&tempOneStudData.score), sizeof(tempOneStudData.score));
				DBIO.read((char*)(&tempOneStudData.advisorID), sizeof(tempOneStudData.advisorID));
				
				int useHashingPrefix;
				
				hashIO.clear();
				hashIO.seekg(0);
				hashIO.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
				
				cout << findHashValue(tempOneStudData.studentID, useHashingPrefix) << " ";
				cout << tempOneStudData.name << " ";
				cout <<tempOneStudData.studentID << " ";
				cout << tempOneStudData.score << " ";
				cout << tempOneStudData.advisorID << "\n";
			}*/
		}
		
		//이위로는 학생 이 아래로는 교수 data부분-------------------------------------------------------------------------------------------------------- 
		
		void fileHashAndDBOpenPro() {
			
			ofstream hashIOPro_temp;
			ofstream DBIOPro_temp;
			
			hashIOPro_temp.open("Professors.hash", ios::out | ostream::binary);
			hashIOPro_temp.close();
				
			hashIOPro.open("Professors.hash", ios::in | ios::out | ostream::binary);
			
			hashIOPro.seekp(0);
			int firstHashTablePrefix = 0;
			int hashPointBlockNum = 0;
			hashIOPro.write((char*)(&firstHashTablePrefix), sizeof(firstHashTablePrefix));
			hashIOPro.write((char*)(&hashPointBlockNum), sizeof(hashPointBlockNum));
			
			DBIOPro_temp.open("Professors.DB", ios::out | ostream::binary);
			DBIOPro_temp.close();
			
			DBIOPro.open("Professors.DB", ios::in | ios::out | ostream::binary);
			
		}
		
		void readProTableAndUpdateFile() { //자료들을 한줄씩 읽음 
			ifstream readProFile("prof_data.csv");
			//input student number by using getline
			string tmp_professorNum;
			getline(readProFile, tmp_professorNum, '\n');
			professorNum = atoi(tmp_professorNum.c_str());
			
			ProfessorData currentProData;
			int useHashingValue;
			int useHashingPrefix;
			int thisBlockNumber;
			//input all studentData by using getline
			
			string tmp_name, tmp_profID, tmp_salary;
			while(getline(readProFile, tmp_name,',')&&getline(readProFile, tmp_profID,',')&&getline(readProFile,tmp_salary,'\n')) {
				strcpy(currentProData.name,tmp_name.c_str());
				if(tmp_name.length() >= 20) {
					string tmp_re_name = tmp_name.substr(0, 19);
					strcpy(currentProData.name,tmp_re_name.c_str());
				}
				currentProData.profID = atoi(tmp_profID.c_str());
				currentProData.salary = atoi(tmp_salary.c_str());
				
				hashIOPro.clear();
				hashIOPro.seekg(0);
				hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
				//useHashingPrefix = useHashingPrefix;
				
				useHashingValue = findHashValue(currentProData.profID, useHashingPrefix);
				thisBlockNumber = findBlockNumberPro(useHashingValue);
				
				inputDataInDBPro(currentProData, thisBlockNumber);
				
			}
			hashTablePrintPro();
			
			readProFile.close();
			hashIOPro.close();
			DBIOPro.close();
			
		}
		
		int findBlockNumberPro(int useHashingValue) {
			int thisBlockNumber = 0;
			
			if(useHashingValue == -1) {
				useHashingValue == 0;
			}
			hashIOPro.clear();
			hashIOPro.seekg(4 + (4 * useHashingValue));
			hashIOPro.read((char*)(&thisBlockNumber), sizeof(thisBlockNumber));
			//cout << thisBlockNumber;
			return thisBlockNumber;
		}
		
		void inputDataInDBPro(ProfessorData currentProData, int thisBlockNumber) {
			
			bool isOverflow = overflowCheckPro(thisBlockNumber, sizeof(currentProData));
			
			if(!isOverflow) {
				inputDataPro(currentProData, thisBlockNumber);
			
			} else {
				
				solutionOverflowPro(currentProData, thisBlockNumber);
			}
		}
		
		bool overflowCheckPro(int thisBlockNumber, int dataSize) {
			
			bool isOverflow = true;
			
			char isNull;
			DBIOPro.clear();
			DBIOPro.seekg((thisBlockNumber + 1) * 4096 - dataSize - 8);
			
			DBIOPro.read((char*)(&isNull), sizeof(isNull));
			
			if(isNull == NULL || DBIOPro.tellg() == -1) {
				isOverflow = false;
			}
			
			return isOverflow;
		}
		
		void inputDataPro(ProfessorData currentProData, int thisBlockNumber) {			
			char isNull;
			int inputLocation;
			
			for(int i = 0; i < 4096; i += sizeof(currentProData)) {
				DBIOPro.clear();
				DBIOPro.seekg((thisBlockNumber * 4096) + i);
				DBIOPro.read((char*)(&isNull), sizeof(isNull));
				
				
				if(isNull == NULL || DBIOPro.tellg() == -1) {
					inputLocation = i;
					//cout << i << " ";
					break;
				}
			}
			
			DBIOPro.clear();
			DBIOPro.seekp(thisBlockNumber * 4096 + inputLocation);
			//cout << currentStudData.name << " ";
			DBIOPro.write((char*)(&currentProData.name), sizeof(currentProData.name));
			DBIOPro.write((char*)(&currentProData.profID), sizeof(currentProData.profID));
			DBIOPro.write((char*)(&currentProData.salary), sizeof(currentProData.salary));
			
		}
		
		void solutionOverflowPro(ProfessorData currentProData, int thisBlockNumber) {
			//hashtable에서 현재 blocknumber와 같은것 찾음 - 함수만들기 
			int blockPointCount = findBlockPointCountPro(thisBlockNumber);
			//cout << blockPointCount << " ";
			//1개면 hashtable을 두배로 늘린뒤 데이터를 다 들고와서 hashPrefix재설정후(prefix1증가) 
			//block을 하나 더 만들어서 재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow를 확인후(재귀호출) 현재 데이터 삽입
			if(blockPointCount == 1) {
				reallocationOnePointPro(currentProData, thisBlockNumber); 
				//cout << "onepointover" <<endl; 
			} else {
				//2개, 4개, 8개 ...면 hashtable을 나눌 필요없이 hashPrefix를 적절히 보고  block을 늘릴 필요없이 데이터를 다 들고와서
				//재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
				//다시 넣을 곳의 block의 overflow확인후 (재귀호출 이때는 point가 한개가 될수있음) 현재 데이터 삽입 
				reallocationMultiPointPro(currentProData, thisBlockNumber);
				//cout << blockPointCount << "multipoint over" << endl;
			}
			
			
		}
		
		int findBlockPointCountPro(int thisBlockNumber) {
			int blockPointCount = 0;
			int useHashingPrefix;
			
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			//useHasingPrefix가 2의 지수승번으로  검사하면됨 
			int tempBlockNum;
			int howManyHashBlock = calculatePow(useHashingPrefix);
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.read((char*)(&tempBlockNum), sizeof(tempBlockNum));
				if(tempBlockNum == thisBlockNumber) {
					blockPointCount++;
				}
			}
			
			return blockPointCount;
		}
		
		void reallocationOnePointPro(ProfessorData currentProData, int thisBlockNumber) {
			//1개면 hashtable을 두배로 늘린뒤 데이터를 다 들고와서 hashPrefix재설정후(prefix1증가) 
			//block을 하나 더 만들어서(실재로 더 만들 필요는 없음 맨끝4k에 추가 관련 함수 필요) 재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow를 확인후(재귀호출) 현재 데이터 삽입
			int finalBlockNum;
			finalBlockNum = findFinalBlockNumPro();
			//cout << finalBlockNum;
			makeHashTableDoublePro();
			
			reallocateBlocksMultiPro(currentProData, thisBlockNumber, finalBlockNum);
			reallocateHashTablePro(currentProData, thisBlockNumber, finalBlockNum);
			
			
		}
		
		void makeHashTableDoublePro() {
			int beforeUseHashingPrefix;
			int afterUseHasingPrefix;
			vector <int> tempHashTable;
			
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&beforeUseHashingPrefix), sizeof(beforeUseHashingPrefix));
			
			int howManyHashBlock = calculatePow(beforeUseHashingPrefix);
			
			int tempHashTableNum;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				tempHashTable.push_back(tempHashTableNum);
			}
			
			hashIOPro.clear();
			hashIOPro.seekp(4 + howManyHashBlock * 4);
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.write((char*)(&tempHashTable[i]), sizeof(tempHashTable[i]));
			}
			
			hashIOPro.clear();
			afterUseHasingPrefix = beforeUseHashingPrefix + 1;
			hashIOPro.seekp(0);
			hashIOPro.write((char*)(&afterUseHasingPrefix), sizeof(afterUseHasingPrefix));
			
			
		}
		
		int findFinalBlockNumPro() {
			int finalBlockNum = 0;
			int useHashingPrefix;
			
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			int howManyHashBlock = calculatePow(useHashingPrefix);
			
			int tempHashTableNum;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				if(finalBlockNum < tempHashTableNum) {
					finalBlockNum = tempHashTableNum;
				}
			}
			//cout << "마지막 block : " << finalBlockNum << endl;
			return finalBlockNum; 
		}
		
		
		void reallocateHashTablePro(ProfessorData currentProData, int thisBlockNumber, int finalBlockNum) {
			int useHashingPrefix;
			int increaseFinalBlockNum = finalBlockNum + 1;
			vector <int> tempChangeHashTable;
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			int howManyHashBlock = calculatePow(useHashingPrefix);
			
			int tempHashTableNum;
			int changeHashPointNum = 0;
			int count = 0;
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
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
				hashIOPro.clear();
				hashIOPro.seekp(4 + tempChangeHashTable[i] * 4);
				hashIOPro.write((char*)(&increaseFinalBlockNum), sizeof(increaseFinalBlockNum));
			}
			
			
			
		}
		
		void reallocationMultiPointPro(ProfessorData currentProData, int thisBlockNumber) {
			//2개, 4개, 8개 ...면 hashtable을 나눌 필요없이 hashPrefix를 적절히 보고  block을 늘릴 필요없이 데이터를 다 들고와서
			//재분배시킨뒤에 hashtable의 값을 block에 따라 바꿔주고
			//다시 넣을 곳의 block의 overflow확인후 (재귀호출 이때는 point가 한개가 될수있음) 현재 데이터 삽입
			
			int finalBlockNum;
			finalBlockNum = findFinalBlockNumPro();
			//cout << finalBlockNum;
			
			reallocateBlocksMultiPro(currentProData, thisBlockNumber, finalBlockNum);
			reallocateHashTablePro(currentProData, thisBlockNumber, finalBlockNum);
			
		}
		
		void reallocateBlocksMultiPro(ProfessorData currentProData, int thisBlockNumber, int finalBlockNum) {
			ProfessorData tempOneProData;
			//StudentData emptyStudData;
			char isNull;
			char tempNullChar[20] = {NULL};
			unsigned int tempNullInt = NULL;
			int tempNullInt_sign = NULL;
			
			
			for(int i = 0; i < 4096; i += sizeof(currentProData)) {
			//for(int i = 0; i < 4096; i ++) {
				DBIOPro.clear();
				DBIOPro.seekg((thisBlockNumber * 4096) + i);
				DBIOPro.read((char*)(&isNull), sizeof(isNull));
				//cout << isNull << "구분자";
				
				if(isNull == NULL || DBIOPro.tellg() == -1) {
					break;
				}
				
				DBIOPro.clear();
				DBIOPro.seekg((thisBlockNumber * 4096) + i);
				DBIOPro.read((char*)(&tempOneProData.name), sizeof(tempOneProData.name));
				DBIOPro.read((char*)(&tempOneProData.profID), sizeof(tempOneProData.profID));
				DBIOPro.read((char*)(&tempOneProData.salary), sizeof(tempOneProData.salary));
				tempProfessorDatas.push_back(tempOneProData);
				
				DBIOPro.clear();
				DBIOPro.seekp(thisBlockNumber * 4096 + i);
				//cout << currentStudData.name << " ";
				DBIOPro.write((char*)(&tempNullChar), sizeof(tempNullChar));
				DBIOPro.write((char*)(&tempNullInt), sizeof(tempNullInt));
				DBIOPro.write((char*)(&tempNullInt_sign), sizeof(tempNullInt_sign));
				
			}
			
			int useHashingPrefix;
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			int blockPointCount = findBlockPointCountPro(thisBlockNumber);
		
			int forRealusedPrefix = -1; //포인터의 개수가 1개면 안빼고 2개면 1빼고 4개면 2빼고 ... 
			while(blockPointCount != 1) {
				forRealusedPrefix ++;
				blockPointCount = blockPointCount/2;
				
			}
			
			int beforeHashValue = findHashValue(currentProData.profID, useHashingPrefix - 1 - forRealusedPrefix);
			
			for(int i = 0; i < tempProfessorDatas.size(); i ++) {
				int newHashValue = findHashValue(tempProfessorDatas[i].profID, useHashingPrefix - forRealusedPrefix);
				if(newHashValue == beforeHashValue) {
					inputDataPro(tempProfessorDatas[i], thisBlockNumber);
				} else {
					inputDataPro(tempProfessorDatas[i], finalBlockNum + 1);
				}
			}
			
			tempProfessorDatas.clear();
			
			int thisHashValue = findHashValue(currentProData.profID, useHashingPrefix - forRealusedPrefix);
			if(thisHashValue == beforeHashValue) {
				inputDataPro(currentProData, thisBlockNumber);
			} else {
				inputDataPro(currentProData, finalBlockNum + 1);
			}
				
		}
		
		
		
		void hashTablePrintPro() {
			int useHashingPrefix;
			
			hashIOPro.clear();
			hashIOPro.seekg(0);
			hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			int howManyHashBlock = calculatePow(useHashingPrefix);
			cout << "-----Professor hashtable-----" << endl;
			cout << "hashTablePrefix :" << useHashingPrefix << endl;
			int tempHashTableNum;
		
			for(int i = 0; i < howManyHashBlock; i++) {
				hashIOPro.read((char*)(&tempHashTableNum), sizeof(tempHashTableNum));
				cout << tempHashTableNum << endl;
			}
			
			cout << "-----hashtableEnd-----" << endl;
			
			/*int count = 0;
			for(int i = 0; i < 4096; i += 28) {
				char isNull;
				DBIOPro.clear();
				DBIOPro.seekg(i + 4096 * 1);
				DBIOPro.read((char*)(&isNull), sizeof(isNull));
				//cout << isNull << "구분자";
				
				if(isNull == NULL || DBIOPro.tellg() == -1) {
					break;
				}
				ProfessorData tempOneProData;
				cout << count << " ";
				count ++;
				
				DBIOPro.clear();
				DBIOPro.seekg(i + 4096 * 1);
				DBIOPro.read((char*)(&tempOneProData.name), sizeof(tempOneProData.name));
				DBIOPro.read((char*)(&tempOneProData.profID), sizeof(tempOneProData.profID));
				DBIOPro.read((char*)(&tempOneProData.salary), sizeof(tempOneProData.salary));
				
				int useHashingPrefix;
				
				hashIOPro.clear();
				hashIOPro.seekg(0);
				hashIOPro.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
				
				cout << findHashValue(tempOneProData.profID, useHashingPrefix) << " ";
				cout << tempOneProData.name << " ";
				cout <<tempOneProData.profID << " ";
				cout << tempOneProData.salary << "\n";
			}*/
		}
		
		void readQueryFile() {
			
			int howManyQuery;
			string whatQuery; //Search-Exact, Search-Range, Join 
			
			ifstream queryPickFile("query.csv");
		
			string tmp_howManyQuery;
			getline(queryPickFile, tmp_howManyQuery, '\n');
			howManyQuery = atoi(tmp_howManyQuery.c_str());
			
			//cout << howManyQuery << endl;
			cout << "------------start query --------------------\n";
			for(int i = 0; i < howManyQuery; i++) {
				getline(queryPickFile, whatQuery,',');
				string tableName, fieldName, temp_ID, temp_lowScore, temp_highScore, temp_lowSalary, temp_highSalary ;
				string joinTable1, joinTable2;
				unsigned int ID;
				float lowScore, highScore;
				int lowSalary, highSalary;
				//cout << whatQuery << " ";
				if(whatQuery.compare("Search-Exact") == 0) {
					getline(queryPickFile, tableName,',');
					getline(queryPickFile, fieldName,',');
					getline(queryPickFile, temp_ID,'\n');
					ID = atoi(temp_ID.c_str());
					
					//cout << tableName << " " << fieldName << " " << ID << endl;
					if((tableName.compare("Professors") == 0 && fieldName.compare("ProfID") == 0) || (tableName.compare("Students") == 0 && fieldName.compare("StudentID") == 0)) {
						exactQuery(tableName, ID);
					} else {
						cout << "wrong input query \n";
					}
					
				} else if(whatQuery.compare("Search-Range") == 0) {
					getline(queryPickFile, tableName,',');
					getline(queryPickFile, fieldName,',');
					if(tableName.compare("Professors") == 0 && fieldName.compare("Salary") == 0) {
						getline(queryPickFile, temp_lowScore,',');
						getline(queryPickFile, temp_highScore,'\n');
						lowSalary = atoi(temp_lowSalary.c_str());
						highSalary = atoi(temp_highSalary.c_str());
						
						searchProfRange(lowSalary, highSalary);
					}
						
					if(tableName.compare("Students") == 0 && fieldName.compare("Score") == 0){
						getline(queryPickFile, temp_lowScore,',');
						getline(queryPickFile, temp_highScore,'\n');
						lowScore = atof(temp_lowScore.c_str());
						highScore = atof(temp_highScore.c_str());
						
						searchStudRange(lowScore, highScore);
					}
					
					//cout << tableName << " " << fieldName << " " << lowScore << " " << highScore << endl;
					
				} else if(whatQuery.compare("Join") == 0) {
					getline(queryPickFile, joinTable1,',');
					getline(queryPickFile, joinTable2,',');
					
					if(joinTable1.compare("Professors") == 0 && joinTable2.compare("Students") == 0) {
						doJoin();
					}
				
				} else {
					cout << "not in query statement\n";
				}
			}
			
			queryPickFile.close();

		}
		
		void exactQuery(string tableName, unsigned int ID) {
			if(tableName.compare("Professors") == 0) {
				searchProfExactID(ID);
			} else {
				searchStudExactID(ID);
			}
		}
		
		void searchProfExactID(unsigned int ID) {
			ifstream hashIO_query;
			ifstream DBIO_query;
			
			hashIO_query.open("Professors.hash", ios::in | ios::binary);
			DBIO_query.open("Professors.DB", ios::in | ios::binary);
			
			int useHashingPrefix;
			int hasingValue;
			int blockLocation;
			
			hashIO_query.clear();
			hashIO_query.seekg(0);
			hashIO_query.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			hasingValue = findHashValue(ID, useHashingPrefix);
			
			hashIO_query.clear();
			hashIO_query.seekg(4 + 4 * hasingValue);
			hashIO_query.read((char*)(&blockLocation), sizeof(blockLocation));
			//cout << useHashingPrefix << " "<< blockLocation << endl;
			char isNull;
			unsigned int pointingID;
			int thisDataLocation;
			bool isExist = false;
			
			for(int i = 0; i < 4096; i += 28) {
				DBIO_query.clear();
				DBIO_query.seekg((blockLocation * 4096) + i);
				DBIO_query.read((char*)(&isNull), sizeof(isNull));
				
				if(isNull == NULL || DBIO_query.tellg() == -1) {
					break;
				}
				
				DBIO_query.clear();
				DBIO_query.seekg((blockLocation * 4096) + i + 20);
				DBIO_query.read((char*)(&pointingID), sizeof(pointingID));
				//cout << pointingID << " ";
				if(pointingID == ID) {
					//cout << pointingID;
					isExist = true;
					char name[20];
					unsigned int profID;
					int salary;
					
					DBIO_query.clear();
					DBIO_query.seekg((blockLocation * 4096) + i);
					DBIO_query.read((char*)(&name), sizeof(name));
					DBIO_query.read((char*)(&profID), sizeof(profID));
					DBIO_query.read((char*)(&salary), sizeof(salary));
					
					cout << "Search-Exact Professor " <<  ID << " : " << name << " " << profID << " " << salary << endl;
					
					break;
				}
			}
			
			if(isExist == false) {
				cout << "Search-Exact Professor " << ID << " is not exist" << endl;
			}
			
			hashIO_query.close();
			DBIO_query.close();
		}
		
		void searchStudExactID(unsigned int ID) {
			ifstream hashIO_query;
			ifstream DBIO_query;
			
			hashIO_query.open("Students.hash", ios::in | ios::binary);
			DBIO_query.open("Student.DB", ios::in | ios::binary);
			
			int useHashingPrefix;
			int hasingValue;
			int blockLocation;
			
			hashIO_query.clear();
			hashIO_query.seekg(0);
			hashIO_query.read((char*)(&useHashingPrefix), sizeof(useHashingPrefix));
			
			hasingValue = findHashValue(ID, useHashingPrefix);
			
			hashIO_query.clear();
			hashIO_query.seekg(4 + 4 * hasingValue);
			hashIO_query.read((char*)(&blockLocation), sizeof(blockLocation));
			//cout << useHashingPrefix << " "<< blockLocation << endl;
			char isNull;
			unsigned int pointingID;
			int thisDataLocation;
			bool isExist = false;
			
			for(int i = 0; i < 4096; i += 32) {
				DBIO_query.clear();
				DBIO_query.seekg((blockLocation * 4096) + i);
				DBIO_query.read((char*)(&isNull), sizeof(isNull));
				
				if(isNull == NULL || DBIO_query.tellg() == -1) {
					break;
				}
				
				DBIO_query.clear();
				DBIO_query.seekg((blockLocation * 4096) + i + 20);
				DBIO_query.read((char*)(&pointingID), sizeof(pointingID));
				//cout << pointingID << " ";
				if(pointingID == ID) {
					//cout << pointingID;
					isExist = true;
					char name[20];
					unsigned int studentID;
					float score;
					unsigned int advisorID;
					
					DBIO_query.clear();
					DBIO_query.seekg((blockLocation * 4096) + i);
					DBIO_query.read((char*)(&name), sizeof(name));
					DBIO_query.read((char*)(&studentID), sizeof(studentID));
					DBIO_query.read((char*)(&score), sizeof(score));
					DBIO_query.read((char*)(&advisorID), sizeof(advisorID));
					
					cout << "Search-Exact Student " <<  ID << " : " << name << " " << studentID << " " << score << " " << advisorID << endl;
					
					break;
				}
			}
			
			if(isExist == false) {
				cout << "Search-Exact Student " << ID << " is not exist" << endl;
			}
			
			hashIO_query.close();
			DBIO_query.close();
		}
		
		//block*block outerloof가 작은거 사용 -> outer:professor, inner:student 
		void doJoin() {
			ifstream pro_DBIO_query;
			ifstream stu_DBIO_query;
			
			ofstream join_query;
			
			pro_DBIO_query.open("Professors.DB", ios::in | ios::binary);
			stu_DBIO_query.open("Student.DB", ios::in | ios::binary);
			
			join_query.open("query2.res", ios::out | ios::binary);
			
			//outer professor, inner student -> student의 advisorID와 professor의 ID가같으면 query.res에 저장
			vector <ProfessorData> tempProBlock;
			vector <StudentData> tempStudBlock;
			
			ProfessorData tempOneProfessor;
			StudentData tempOneStudent;
			
			char isProNull;
			char isStudNull;
			int proBlockLocation = 0;
			int studBlockLocation = 0;
			
			while (true) {
				for(int i = 0; i < 4096; i += 28) {
					pro_DBIO_query.clear();
					pro_DBIO_query.seekg((proBlockLocation * 4096) + i);
					pro_DBIO_query.read((char*)(&isProNull), sizeof(isProNull));
				
					if(isProNull == NULL || pro_DBIO_query.tellg() == -1) {
						break;
					}
										
					pro_DBIO_query.clear();
					pro_DBIO_query.seekg((proBlockLocation * 4096) + i);
					pro_DBIO_query.read((char*)(&tempOneProfessor.name), sizeof(tempOneProfessor.name));
					pro_DBIO_query.read((char*)(&tempOneProfessor.profID), sizeof(tempOneProfessor.profID));
					pro_DBIO_query.read((char*)(&tempOneProfessor.salary), sizeof(tempOneProfessor.salary));
					tempProBlock.push_back(tempOneProfessor);
					//cout << tempOneProfessor.profID;
				}
				
				proBlockLocation ++;
				if(pro_DBIO_query.tellg() == -1) {
					break;
				}
				int i = 0;
				while (true) {
					
					for(int j = 0; j < 4096; j += 32) {
						stu_DBIO_query.clear();
						stu_DBIO_query.seekg((studBlockLocation * 4096) + j);
						stu_DBIO_query.read((char*)(&isStudNull), sizeof(isStudNull));
						
						if(isStudNull == NULL || stu_DBIO_query.tellg() == -1) {
							break;
						}
						
						stu_DBIO_query.clear();
						stu_DBIO_query.seekg((studBlockLocation * 4096) + j);
						stu_DBIO_query.read((char*)(&tempOneStudent.name), sizeof(tempOneStudent.name));
						stu_DBIO_query.read((char*)(&tempOneStudent.studentID), sizeof(tempOneStudent.studentID));
						stu_DBIO_query.read((char*)(&tempOneStudent.score), sizeof(tempOneStudent.score));
						stu_DBIO_query.read((char*)(&tempOneStudent.advisorID), sizeof(tempOneStudent.advisorID));			
						tempStudBlock.push_back(tempOneStudent);
						//cout << tempOneStudent.advisorID << " ";

					}
					
					//block*block join시작 - 순서는 studname, studID, score, advisorID, professorname, salary 
					for(int proBlock = 0; proBlock < tempProBlock.size(); proBlock++) {
						for(int studBlock = 0; studBlock < tempStudBlock.size(); studBlock++) {
							//cout << "do?";
							if(tempProBlock[proBlock].profID == tempStudBlock[studBlock].advisorID) {
								
								join_query.write((char*)(&tempStudBlock[studBlock].name), sizeof(tempStudBlock[studBlock].name));
								join_query.write((char*)(&tempStudBlock[studBlock].studentID), sizeof(tempStudBlock[studBlock].studentID));
								join_query.write((char*)(&tempStudBlock[studBlock].score), sizeof(tempStudBlock[studBlock].score));
								join_query.write((char*)(&tempStudBlock[studBlock].advisorID), sizeof(tempStudBlock[studBlock].advisorID));
								join_query.write((char*)(&tempProBlock[proBlock].name), sizeof(tempProBlock[proBlock].name));
								join_query.write((char*)(&tempProBlock[proBlock].salary), sizeof(tempProBlock[proBlock].salary));
								//cout << "joinstu" << tempStudBlock[studBlock].name << endl;
							}
						}
					}
					
					studBlockLocation ++;
					tempStudBlock.clear();

					if(stu_DBIO_query.tellg() == -1) {
						break;
					}
				}
				studBlockLocation = 0;
				
				tempProBlock.clear();
			}
			
			pro_DBIO_query.close();
			stu_DBIO_query.close();
			join_query.close();
			
		}
		
		void searchProfRange(int lowSalary, int highSalary) {
			
		}
		
		void searchStudRange(float lowScore, float highScore) {
			
			//range_query.open("query.res", ios::out | ios::binary);
			bpt->rangeSearch(lowScore, highScore);
			
			//range_query.close();
		}
		
};

int main(int argc, char** argv) {
	StuAndProFileStruct stuAndProFS;
	stuAndProFS.fileHashAndDBOpen();
	stuAndProFS.readStudTableAndUpdateFile();
	stuAndProFS.fileHashAndDBOpenPro();
	stuAndProFS.readProTableAndUpdateFile();
	
	bpt = new BplusTree();
	bpt->makeStudentB();
 	
	
	stuAndProFS.readQueryFile();
	delete bpt;
	
	//일단 query파일을 불러와서 조건에 따라 함수수행 .. match / range/ join
	
	
	
	 
	return 0;
}
