#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include<iostream>
#include<vector>
#include<cstdlib>
#include <unistd.h>
#include<ctime>
#include<queue>
#include<list>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Input.H>
#include <string>

using namespace std;
//void showMenu();

pthread_mutex_t chopsticks[5];
pthread_t philosophers[5];
int eating_count[5] = { 0 };
//rows indicate num of process and col indicate num of resources
int rows = 20, cols = 20; // default number of rows and columns
int process, resource;
extern "C"
{
    void* philosopher(void* x);
    void philosopherStart();
}
void CPUscedulingStart();

bool safteyalgo(int *available,int **allocate,int**need,bool*fin,int**max);
void resourceReqAlgo(int pID,int *available,int **allocate,int**need,bool*fin,int**max);
void start();
void getInfo(int *available,int **allocate,int**need,bool*fin,int**max);
bool safe();
void deadlockStart();

const int outerpagesize = 4096;//2^12
const int innerpagesize = 1024;//2^10
const int pageoffset = 1024;//2^10
const int hashpagesize = 4096;
void twoLevelPagingStart();
void HashedPTwithTLBstart();

void CircularSecondChanceAlgoStart();

class Process {
public:
    int processId;
    int arrivalTime;
    int burstTime;
    int queueNumber;
    int priority;
    Process() {
        processId = -1;
        arrivalTime = 0;
        burstTime = 0;
        queueNumber = 0;
        priority = 0;
    }
};

struct CompareProcess {
    bool operator()(const Process& p1, const Process& p2) {
        if (p1.arrivalTime == p2.arrivalTime) {
            return p1.processId > p2.processId; // Use processId as tiebreaker
        }
        return p1.arrivalTime > p2.arrivalTime; // Sort based on arrival time
    }
};



void sortQueue(queue<Process>& q);
void takeInputMulti(int* (&allInfo)[3], int& num, Process& prcs, queue<Process>& all, bool usePriority);
void displayOutput(int** allInfo, int& num);
void multiLevel();

class paging
{
public:
	bool valid_bit;
	int framenum;
};
class twolevelpaging
{
	vector<vector<paging>> pt;
	int count;
	//class innerpage
	//{
	//public:
	//	innerpage()
	//	{
	//		for (int i = 0; i < innerpagesize; i++)
	//		{

	//		}
	//	}
	//};

public:
	twolevelpaging()
	{
		pt.resize(outerpagesize);
		for (int i = 0; i < outerpagesize; i++)
		{
			vector<paging> innerpt;
			innerpt.resize(innerpagesize);
			for (int j = 0; j < innerpagesize; j++)
			{
				innerpt[j].valid_bit = 0;
				//innerpt[j].framenum = -1;
			}
			pt[i] = innerpt;
		}
		count = 0;
	}
	int getouterpn(int logicaladdress)
	{
		return  logicaladdress / (outerpagesize * innerpagesize);
	}
	int getinnerpn(int logicaladdress)
	{
		return  (logicaladdress / innerpagesize) % innerpagesize;
	}
	int getphysicaladdress(int logicaladdress)
	{
		int opn = getouterpn(logicaladdress);
		int ipn = getinnerpn(logicaladdress);
		int offset = logicaladdress % innerpagesize;
		int physicaladdress;
		if (!pt[opn][ipn].valid_bit)
		{
			cerr << "\npage fault occured\n";
		}
		cout << "offset: " << offset<<" opn: "<<opn<<" ipn:"<<ipn;
		physicaladdress = pt[opn][ipn].framenum * innerpagesize + offset;
		return physicaladdress;
	}
	int getcount()
	{
		return count;
	}
	void mapping(int fn, int logicaladdress,bool flag=1)
	{
		/*int opn = pn / innerpagesize;
		int ipn = pn % innerpagesize;*/
		int opn = getouterpn(logicaladdress);
		int ipn = getinnerpn(logicaladdress);

		
		if (flag)
		{
			pt[opn][ipn].valid_bit = 1;
			pt[opn][ipn].framenum = fn;
			cout << "\nmapped in pt\n";
		}
			
		else
		{
			pt[opn][ipn].valid_bit = 0;
			cout << "\ndeleted from pt\n";
		}

	}
};


class tlb
{
protected:
	int size;
	list<pair<int, int>> x;

public:
	tlb(int s=10)
	{
		size = s;
	}
	void setsize(int s)
	{
		size = s;
	}
	bool check(int& fn, int pn)
	{
		for (list<pair<int,int>>::iterator it=x.begin(); it!=x.end() ; it++)
		{
			if (it->first==pn)
			{
				fn = it->second;
				x.splice(x.begin(), x, it);
				return 1;
			}
		}
		return 0;
	}

	void insert(int fn, int pn)
	{
		if (x.size()==size)
		{
			x.pop_back();
		}
		x.push_front(make_pair(pn, fn));
	}

	
};

class HashPT:public tlb 
{
	vector<list<pair<int, int>>> PT;
public:
	HashPT(int s):PT(s)
	{
		/*for (int i = 0; i < s; i++)
		{
			PT.push_back(make_pair(-1, -1));
		}*/
		
	}

	void insert(int FN, int PN)
	{
		int index = PN % PT.size();
		for (list<pair<int,int>>::iterator it = PT[index].begin(); it!=PT[index].end() ; it++)
		{
			if (it->first==PN)
			{
				it->second = FN; return;
			}
		}
		PT[index].push_back(make_pair(PN, FN));
	}

	bool check(int& FN, int PN)
	{
		int index = PN % PT.size();
		for (list<pair<int, int>>::iterator it = PT[index].begin(); it != PT[index].end(); it++)
		{
			FN = it->second;
			PT[index].splice(PT[index].begin(), PT[index], it);
			return 1;
		}
		return 0;
	}
	int getFrameNum()
	{
		srand(time(NULL));
		return rand() % 256;
	}
	int getPageNum(int logicalAddress)
	{
		return logicalAddress / PT.size();
	}
	int getOffset(int logicalAddress)
	{
		return logicalAddress % PT.size();
	}
	int getPhysicalAddress(int logicalAddress,tlb obj,HashPT H_obj)
	{
		int offset = getOffset(logicalAddress);
		int PN = getPageNum(logicalAddress);
		int FN = getFrameNum();
		int physicalAddress;

		if (obj.check(FN,PN))
		{
			cout << "\nTLB hit\n";
			physicalAddress = FN * PT.size() + offset;
		}

		else
		{
			cout << "\nTLB miss\n";
			if (H_obj.check(FN,PN))
			{
				cout << "\nPage Table Hit\n";
				physicalAddress = FN * PT.size() + offset;
				cout << "\nInserted in TLB after PT Hit\n";
				obj.insert(FN, PN);
			}
			else
			{
				cout << "\nPage Fault\n";
				H_obj.insert(FN, PN);
				obj.insert(FN, PN);
				physicalAddress = FN * PT.size() + offset;
			}
		}

		return physicalAddress;
		
	}
};


//class hash
//{
//	int size;
//	list<int>* table;
//public:
//	hash(int s = 10)
//	{
//		size = s;
//		table = new list<int>[size];
//	}
//
//	void input(int d)
//	{
//		int x;
//		x = d % size;
//		table[x].push_back(d);
//	}
//
//	void remove(int d)
//	{
//		int x;
//		x = d % size;
//		table[x].remove(d);
//
//	}
//
//	bool search(int d)
//	{
//		int x;
//		x = d % size;
//		for (list<int>::iterator it = table[x].begin(); it != table[x].end(); ++it)
//		{
//			int i = *it;
//			if (i==d)
//			{
//				return 1;
//			}
//			
//		}
//		return 0;
//	}
//
//	void display()
//	{
//		for (int i = 0; i < size; i++)
//		{
//			cout << i << " | ";
//			for (list<int>::iterator it = table[i].begin(); it != table[i].end(); ++it)
//			{
//				int j = *it;
//				cout << " -> " << j;
//			}
//			cout << " \n";
//		}
//	}
//};


class CircularSecondChanceAlgo
{
	class Node
	{
	public:
		int PN;
		bool useBit;
		Node* nextPtr;

		Node(int p)
		{
			PN = p;
			useBit = 0;
			nextPtr = NULL;
		}
	};
	Node* head;
	int pSize;
	int pFault;
	int max;
	void enter(int PN)
	{
		int c=0;
		//while (c==pSize)
		//{
			if (head == NULL)
			{
				//c++;
				head = new Node(PN);
				head->nextPtr = head;
				
			}
			else
			{
				//c++;
				Node* temp = head;
				while (temp->nextPtr != head)
				{
					temp = temp->nextPtr;
				}
				Node* tempPtr = new Node(PN);
				tempPtr->nextPtr = head;
				temp->nextPtr = tempPtr;
			}
		//}
		
	}

	void setUseBitTrue(int PN)
	{
		Node* temp = head;
		while (temp->PN!=PN)
		{
			temp = temp->nextPtr;
		}
		temp->useBit = 1;
	}


	Node* setUseBitsandGetLRU(int PN)
	{
		Node* temp = head;
		while (true)
		{
			if (temp->useBit==1 && temp->PN!=PN)
			{
				temp->useBit = 0;
			}
			else if (temp->useBit == 0 && temp->PN != PN)
			{
				return temp;
			}
			//
			else if (temp->useBit == 0 && temp->PN == PN)
			{
				temp->useBit = 1;
			
			}
			else
			{
				temp->useBit = 0;
			}
			
			temp = temp->nextPtr;

			if (temp==head)
			{
				Node* temp2 = head;
				while (temp2!=NULL && temp2->nextPtr!=head)
				{
					temp2->useBit = 0;
					temp2 = temp2->nextPtr;

				}
			}
		}
	}
public:
	CircularSecondChanceAlgo(int s)
	{
		pSize = s;
		pFault = 0;
		head = NULL;
		max = 0;
	}
	bool isFull() const
	{
		return pSize == max;
	}
	bool isEmpty() const {
		return max == 0;
	}
	void addPagesinList(int PN)
	{
		if (isFull())
		{
			cerr << "\nFULL\n";
			return;
		}
		Node* temp = new Node(PN);
		if (isEmpty())
		{
			head = temp;
			head->nextPtr = head;
		}
		else
		{
			Node* curr = head;
			while (curr->nextPtr != head)
			{
				curr = curr->nextPtr;
			}
			curr->nextPtr = temp;
			temp->nextPtr = head;
		}
		max++;
	}
	void insertAtHead(int PN)
	{
		if (isFull())
		{
			cerr << "\nFULL\n";
			return;
		}

		Node* newNode = new Node(PN);
		if (isEmpty())
		{
			head = newNode;
			head->nextPtr = head;
		}
		else
		{
			newNode->nextPtr = head;
			Node* temp = head;
			while (temp->nextPtr != head) {
				temp = temp->nextPtr;
			}
			temp->nextPtr = newNode;
			head = newNode;
		}
		max++;
	}
	Node* insertInBetween(Node* pos, int PN)
	{
		Node* newNode = new Node(PN);
	
		Node* prevNode = head;
		
		
		while (prevNode->nextPtr != pos)
		{
			prevNode = prevNode->nextPtr;
			
			if (prevNode == head) 
			{
				cout << "Current node not found in the list" << endl;
				return NULL;
			}
		}
		
		prevNode->nextPtr = pos->nextPtr;
		
		newNode->nextPtr = pos->nextPtr;
		pos->nextPtr = NULL;
		delete pos;
		return newNode;
		
	}

	int getFaults()
	{
		return pFault;
	}
	double getFaultProbabilty()
	{
		return pFault / pSize;
	}
	double getFaultPercentage()
	{
		return getFaultProbabilty() * 100;
	}

	void start(vector<int> x)
	{
		for (int i = 0; i < x.size(); i++)
		{
			int PN;
			Node* temp;
			bool flag;
			temp = head;
			PN = x[i];
			flag = 0;

			while (temp != NULL && temp->nextPtr != head)
			{

				if (temp->PN == PN)
				{
					flag = 1;
					setUseBitTrue(PN);
					
					break;

				}
				temp = temp->nextPtr;

			}
			if (!flag)
			{
				int c = 0;
				pFault++;
				if (head == NULL /*|| c!=getPagesInMem()*/)
				{
					addPagesinList(PN);
					c++;
				}
				else if (head->useBit == 0)
				{
					//enter(PN);
					insertAtHead(PN);
					head->PN = PN;
					head->useBit = 0;
					head = head->nextPtr;
				}
				else
				{
					Node* temp = setUseBitsandGetLRU(PN);
					//Node* t=insertInBetween(temp, PN);
					
					temp->PN = PN;
					temp->useBit = 0;
					head = temp->nextPtr;
				}
				cout << "Page fault at reference " << i << ": Page " << PN << endl;
			}
			else
			{
				cout << "Successful reference at " << i << ": Page " << PN << endl;
			}
		}
	}
	int getPagesInMem()
	{
		return pSize;
	}

	void printList()
	{
		if (head == NULL) {
			cout << "List is empty." << endl;
			return;
		}

		Node* curr = head;
		do {
			cout << curr->PN << " ";
			curr = curr->nextPtr;
		} while (curr != head);
		cout << endl;
	}



};

// void load() {
//     cout << "\t\t\t\t\t\tOS Simulator\n";
//     char a = 177;
//     char b = 219;

//     cout << "\n\n\n\t\t\t\tLoading . . . . . ";
//     cout << endl << endl;
//     cout << "\t\t\t\t\t";

//     for (int i = 0; i <= 25; i++)
//         cout << a;
//     usleep(15000000);

//     cout << "\r";
//     cout << "\t\t\t\t\t";

//     for (int i = 0; i <= 25; i++) {
//         cout << b;
//         fflush(stdout);
//         usleep(20000000);
//     }
// }
// int main()
// {
//     //load();
//     //system("cls");
    
//     cout<<"\n\t\t\tChoose:\n";
//     cout<<"1.CPU scheduling\n2.Semaphore(Dining Philosopher)\n3.Deadlock(Banker's Algo)\nMemory Management: 4.Two Level Page Table  5.Hashed Page Table with TLB\n6.Page Replacement(Circular Second Chance Algo)\n";
//     showMenu();
//     return 0;
// }

// void showMenu()
// {
//      int choice;
//  cout<<"\nEnter your choice: ";
//  cin>>choice;

//  do {
//     switch (choice) {
//         case 1: {
//             CPUscedulingStart();
//             break;
//         }
//         case 2: {
//             philosopherStart();
//             break;
//         }
//         case 3: {
//             deadlockStart();
//             break;
//         }
//         case 4: {
//             twoLevelPagingStart();
//             break;
//         }
//         case 5: {
//             HashedPTwithTLBstart();
//             break;
//         }
//         case 6: {
//             CircularSecondChanceAlgoStart();
//             break;
//         }
//         case 0: {
//             cout << "\nTerminated\n";
//             break;
//         }
//         default: {
//             cout << "\nERROR!!!\n";
//             break;
//         }
//     }
//     cout << "\nEnter 0 to exit or any other number to continue > ";
//     cin >> choice;
// } while (choice != 0);

 

// }



void showMenu(Fl_Widget* widget, void* data) {
    Fl_Window* window = (Fl_Window*)data;
    window->color(FL_GREEN);

        Fl_Menu_Bar* menu = new Fl_Menu_Bar(0, 0, 600, 30);
    menu->add("File/Save", FL_CTRL + 's', NULL, NULL, FL_MENU_DIVIDER);
    menu->add("File/Quit", FL_CTRL + 'q', [](Fl_Widget* widget, void* data) {
        Fl::delete_widget((Fl_Widget*)data);
    }, window);
    
    // Create a text display widget to show the menu and output
    Fl_Text_Buffer* buffer = new Fl_Text_Buffer();
    Fl_Text_Display* display = new Fl_Text_Display(20, 20, 580, 300);
    display->buffer(buffer);
    buffer->text("Choose:\n1.CPU scheduling\n2.Semaphore(Dining Philosopher)\n3.Deadlock(Banker's Algo)\nMemory Management: 4.Two Level Page Table  5.Hashed Page Table with TLB\n6.Page Replacement(Circular Second Chance Algo)\n");

    // Create buttons for each menu item
    Fl_Button* btn1 = new Fl_Button(10, 320, 580, 30, "CPU scheduling");
    btn1->callback([](Fl_Widget* widget, void* data) {
        CPUscedulingStart();
    }, NULL);

    Fl_Button* btn2 = new Fl_Button(10, 360, 580, 30, "Semaphore(Dining Philosopher)");
    btn2->callback([](Fl_Widget* widget, void* data) {
        philosopherStart();
    }, NULL);

    Fl_Button* btn3 = new Fl_Button(10, 400, 580, 30, "Deadlock(Banker's Algo)");
    btn3->callback([](Fl_Widget* widget, void* data) {
        deadlockStart();
    }, NULL);

    Fl_Button* btn4 = new Fl_Button(10, 440, 580, 30, "Two Level Page Table");
    btn4->callback([](Fl_Widget* widget, void* data) {
        twoLevelPagingStart();
    }, NULL);

    Fl_Button* btn5 = new Fl_Button(10, 480, 580, 30, "Hashed Page Table with TLB");
    btn5->callback([](Fl_Widget* widget, void* data) {
        HashedPTwithTLBstart();
    }, NULL);

    Fl_Button* btn6 = new Fl_Button(10, 520, 580, 30, "Circular Second Chance Algo");
    btn6->callback([](Fl_Widget* widget, void* data) {
        CircularSecondChanceAlgoStart();
    }, NULL);

    // Add all widgets to the window and show it
    window->add(display);
    window->add(btn1);
    window->add(btn2);
    window->add(btn3);
    window->add(btn4);
    window->add(btn5);
    window->add(btn6);
    // window->add(btn0);

    window->show();

// Run the event loop until the window is closed
Fl::run();
}
int main() {

Fl_Window* window = new Fl_Window(620, 580, "OS Simulator----Menu");

showMenu(NULL, (void*)window);


delete window;
return 0;

}

void* philosopher(void* x) {
    int philosopher_id = *(int*)x;
    int left_chopstick = philosopher_id;
    int right_chopstick = (philosopher_id + 1) % 5;
    int count = 0;

    while (count < 3) {


        pthread_mutex_lock(&chopsticks[left_chopstick]);
        printf("Philosopher %d has picked up left chopstick %d\n", philosopher_id + 1, left_chopstick + 1);


        int right_locked = pthread_mutex_trylock(&chopsticks[right_chopstick]);

        if (right_locked == 0) {

            printf("Philosopher %d has picked up right chopstick %d\n", philosopher_id + 1, right_chopstick + 1);


            printf("Philosopher %d is eating using chopsticks %d and %d\n", philosopher_id + 1, left_chopstick + 1, right_chopstick + 1);
            eating_count[philosopher_id]++;
            count++;


            pthread_mutex_unlock(&chopsticks[right_chopstick]);
            pthread_mutex_unlock(&chopsticks[left_chopstick]);

            printf("Philosopher %d is thinking\n", philosopher_id + 1);
            sleep(1);
        }
        else {
            pthread_mutex_unlock(&chopsticks[left_chopstick]);
        }

    }

    pthread_exit(NULL);
}

void philosopherStart() 
{
    int i;
    int philosopher_ids[5];
    for (i = 0; i < 5; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
    }

    // Create philosopher threads
    for (i = 0; i < 5; i++) {
        philosopher_ids[i] = i;
        pthread_create(&philosophers[i], NULL, philosopher, &philosopher_ids[i]);
    }

    for (i = 0; i < 5; i++) {
        pthread_join(philosophers[i], NULL);
    }

    for (i = 0; i < 5; i++) {
        printf("Philosopher %d ate %d times.\n", i + 1, eating_count[i]);
    }

    }



void deadlockStart()
{
    int *available = new int[cols];
    int **allocate = new int *[rows];
    int **need = new int *[rows];
    bool *fin = new bool[rows];
    int **max = new int *[rows];
    start();
     for (int i = 0; i < rows; i++)
    {
        allocate[i] = new int[cols];
        need[i] = new int[cols];
        max[i] = new int[cols];
    }

    cout << "\nEnter available resources: \n";
    for (int i = 0; i < resource; i++)
    {
        cin >> available[i];
    }
    cout << "\nEnter allocated resources: \n";
    for (int i = 0; i < process; i++)
    {
        for (int j = 0; j < resource; j++)
        {
            cin >> allocate[i][j];
        }
    }
    cout << "\nEnter max matrix: " << endl;
    for (int i = 0; i < process; i++)
    {
        for (int j = 0; j < resource; j++)
        {
            cin >> max[i][j];
            need[i][j] = max[i][j] - allocate[i][j];
        }
    }
    int i;
    cout<<"\nEnter\n1.For saftey algo\n2.Resource Request\n";
    cin>>i;
    switch (i)
    {
    case 1:
        {
           if (safteyalgo(available,allocate,need,fin,max))
    {
        cout << "\nSystem is in safe state\n";
    }
    else
    {
        cout << "\nSystem isn't in safe state\n";
    } 
        break;
        }
    
    case 2:
    {
        getInfo(available,allocate,need,fin,max);
        break;
    }
    default:
    cout<<"\nERROR!!!\n";
        break;
    }
    
    
      delete[] available;
    for (int i = 0; i < rows; i++)
    {
        delete[] allocate[i];
        delete[] need[i];
        delete[] max[i];
    }
    delete[] allocate;
    delete[] need;
    delete[] max;
    delete[] fin;
    


}

void start()
{
    cout << "\nEnter number of processes and resources: \n";
    cin >> process >> resource;

    if (process > 20 || resource > 20)
    {
        cout << "\nEnter new rows and columns: ";
        cin >> rows >> cols;
    }
}
void getInfo(int *available,int **allocate,int**need,bool*fin,int**max)
{
    int pID;
        cout << "\nEnter process ID to request resources: ";
         cin>>pID;
         resourceReqAlgo(pID,available,allocate,need,fin,max);
    

}
bool safteyalgo(int *available,int **allocate,int**need,bool*fin,int**max)
{
    
    vector<int> ss; // safe sequence
    int *current = new int[cols];
    bool check;

   

    for (int i = 0; i < resource; i++)
    {
        current[i] = available[i];
    }
    for (int i = 0; i < process; i++)
    {
        fin[i] = false;
    }

    do
    {
        check = false;
        for (int i = 0; i < process; i++)
        {
            if (!fin[i])
            {
                bool x = true;
                for (int y = 0; y < resource; y++)
                {
                    if (need[i][y] > current[y])
                    {
                        x = false;
                        break;
                    }
                }
                if (x)
                {
                    for (int k = 0; k < resource; k++)
                    {
                        current[k] += allocate[i][k];
                    }
                    fin[i] = true;
                    ss.push_back(i);
                    check = true;
                }
            }
        }
    } while (check);

  delete[] current;

    if (ss.size() == process)
    {
        cout <<"The safe sequence is: ";
for (int i = 0; i < ss.size(); i++)
{
cout << "P" << ss[i];
if (i != ss.size() - 1)
{
cout << " -> ";
}
}
return true;
}
else
{
return false;
}
}


void resourceReqAlgo(int pID,int *available,int **allocate,int**need,bool*fin,int**max)
{
    int req[resource];
    bool flag=1;

    cout<<"\nEnter resource to be requested for P"<<pID<<" :";
    for (int i = 0; i < resource; i++)
    {
        cin>>req[i];
    }


    for (int i = 0; i < resource; i++)
    {
        if(req[i]>need[pID][i] || req[i]>available[i])
        {
            flag=0;
            break;
        }
    }
    if (flag)
    {
        for (int i = 0; i < resource; i++)
        {
           available[i]-=req[i];
           allocate[pID][i]+=req[i];
           need[pID][i]-=req[i];
        }

        if(safteyalgo(available,allocate,need,fin,max))
        {
          cout<<endl<<"Resource request successfull for P"<<pID;
          cout<<"\n";
        }
        else
        {
            cout<<endl<<"Resource request unsuccessfull for P"<<pID;
          cout<<"\n";

          for (int i = 0; i < resource; i++)
        {
           available[i]+=req[i];
           allocate[pID][i]-=req[i];
           need[pID][i]+=req[i];
        }
        }
        
    }
    else
        {
            cout<<endl<<"Resource request unsuccessfull for P"<<pID;
          cout<<"\n";
        }
    
    
}

void twoLevelPagingStart()
{
	twolevelpaging pt_obj;
	
	//srand(time(null));
	//unsigned int logicaladdress = static_cast<unsigned int>(rand()) << 16 | rand();
	unsigned int logicaladdress = 8196;
	//cout << logicaladdress;
	int fn = rand() % (1ul << 20);//4kb page size
	//int pn = logicaladdress / innerpagesize;
 //   
	//pt_obj.mapping(1, logicaladdress);
	pt_obj.mapping(fn, logicaladdress);
	/*pt_obj.mapping(0, 0);
	pt_obj.mapping(1, 1);
	pt_obj.mapping(2, 2);
	pt_obj.mapping(3, 3);*/

	int physicaladdress = pt_obj.getphysicaladdress(logicaladdress);
	cout << "\nlogical address: " << logicaladdress << " \nphysical address: " << physicaladdress << " \n";

	//cout << pt_obj.getcount();

	//twolevelpaging pt_obj;

	//// map some pages
	//pt_obj.mapping(0,0);
	//pt_obj.mapping(1,1);
	//pt_obj.mapping(2,2);
	//pt_obj.mapping(3,3);

	//// generate logical address
	//unsigned int logicaladdress = 0;
	//int pageoffset = rand() % pageoffset; // generate a random page offset
	//int pagenumber = rand() % (outerpagesize * innerpagesize); // generate a random page number

	//// set the logical address to the selected page number and offset
	//logicaladdress = pagenumber * pageoffset + pageoffset;

	//int physicaladdress = pt_obj.getphysicaladdress(logicaladdress);
	//cout << "\nlogical address: " << logicaladdress << " \nphysical address: " << physicaladdress << " \n";
	
}

void HashedPTwithTLBstart()
{
	
	HashPT hashPT(16);

	
	tlb TLB(8);

	
	srand(time(NULL));
	
		int logicalAddress = rand() % 4096;

		
		int physicalAddress = hashPT.getPhysicalAddress(logicalAddress, TLB, hashPT);

		cout << "Logical address: " << logicalAddress << " Physical address: " << physicalAddress << endl;
		int logicalAdd = rand() % 4096;
		int physicalAd = hashPT.getPhysicalAddress(logicalAdd, TLB, hashPT);

		cout << "Logical address: " << logicalAdd << " Physical address: " << physicalAd << endl;

}


void CircularSecondChanceAlgoStart()
{
	vector<int>x = { 0 ,4, 1, 4, 2, 4, 3, 4, 2, 4, 0, 4, 1, 4, 2, 4, 3, 4 };
	cout << x.size() << endl;
	

	CircularSecondChanceAlgo obj(3);
	obj.start(x);

	cout << "\n";
	cout << "Total page faults: " << obj.getFaults() << endl;
    cout << "Total page fault probabilty: " << obj.getFaultProbabilty() << endl;
    cout << "Total page fault percentage:  " << obj.getFaultPercentage() << endl;
	//cout << "\n LIst: "; obj.printList();

	
}

void sortQueue(queue<Process>& q) {
    priority_queue<Process, vector<Process>, CompareProcess> sortedQueue;

    while (!q.empty()) {
        sortedQueue.push(q.front());
        q.pop();
    }

    while (!sortedQueue.empty()) {
        q.push(sortedQueue.top());
        sortedQueue.pop();
    }
}

void takeInputMulti(int* (&allInfo)[3], int& num, Process& prcs, queue<Process>& all, bool usePriority) {
    cout << "\nEnter number of processes > ";
    cin >> num;

    for (int i = 0; i < 3; i++) {
        allInfo[i] = new int[num];
    }

    for (int i = 0; i < num; i++) {

        prcs.processId = i + 1;

        do {
            cout << "\nEnter Arrival time for Process ID. " << i + 1 << " > ";
            cin >> prcs.arrivalTime;
        } while (prcs.arrivalTime < 0);
        allInfo[0][i] = prcs.arrivalTime;

        do {
            cout << "\nEnter Burst time for Process ID. " << i + 1 << " > ";
            cin >> prcs.burstTime;
        } while (prcs.burstTime < 0);
        allInfo[1][i] = prcs.burstTime;

        do {
            cout << "\nEnter Queue no. for Process ID. " << i + 1 << " > ";
            cin >> prcs.queueNumber;
        } while (prcs.queueNumber < 1 || prcs.queueNumber > 3);

        if (prcs.queueNumber == 1 && usePriority) {
            do {
                cout << "\nEnter Priority for Process ID. " << i + 1 << " > ";
                cin >> prcs.priority;
            } while (prcs.priority < 0);
        }

        all.push(prcs);
    }

    sortQueue(all);
}

void displayOutput(int** allInfo, int& num) {
    cout << "\n\nProcess ID   Arrival Time   Burst Time   Completion Time   Waiting Time   Turnaround Time\n";
    for (int i = 0; i < num; i++) {
        cout << "  " << i + 1 << "             " << allInfo[0][i] << "             " << allInfo[1][i] << "             " << allInfo[2][i];
        cout << "                    " << allInfo[2][i] - allInfo[1][i] - allInfo[0][i] << "                    " << allInfo[2][i] - allInfo[0][i] << endl;
    }
}

void multiLevel() {
    int num;
    int* all_info[3];
    queue<Process> q1, q2, q3;
    Process currentProcess;
    int currentTime = 0;

    bool q1Running = false;
    bool q2Running = false;
    bool q3Running = false;

    takeInputMulti(all_info, num, currentProcess, q1, true);

    while (!q1.empty() || !q2.empty() || !q3.empty()) {

        if (!q1.empty() && !q1Running) {
            currentProcess = q1.front();
            q1.pop();
            q1Running = true;
        }
        else if (!q2.empty() && !q2Running) {
            currentProcess = q2.front();
            q2.pop();
            q2Running = true;
        }
        else if (!q3.empty() && !q3Running) {
            currentProcess = q3.front();
            q3.pop();
            q3Running = true;
        }

        currentProcess.burstTime--;

        if (currentProcess.burstTime == 0) {
            all_info[2][currentProcess.processId - 1] = currentTime + 1;

            if (currentProcess.queueNumber == 1) {
                q1Running = false;
            }
            else if (currentProcess.queueNumber == 2) {
                q2Running = false;
            }
            else {
                q3Running = false;
            }
        }
        else if (currentProcess.queueNumber == 1 && currentProcess.priority > 0) {
            currentProcess.priority--;
            q2.push(currentProcess);
            q1Running = false;
        }
        else if (currentProcess.queueNumber == 2) {
            q3.push(currentProcess);
            q2Running = false;
        }
        else {
            if (currentProcess.queueNumber == 1) {
                q1.push(currentProcess);
            }
            else if (currentProcess.queueNumber == 2) {
                q2.push(currentProcess);
            }
            else {
                q3.push(currentProcess);
            }

            if (q1Running) {
                q1.pop();
                q1Running = false;
            }
            else if (q2Running) {
                q2.pop();
                q2Running = false;
            }
            else {
                q3.pop();
                q3Running = false;
            }
        }

        if (!q1.empty() && currentTime >= q1.front().arrivalTime && !q1Running) {
            sortQueue(q1);
        }

        if (!q2.empty() && currentTime >= q2.front().arrivalTime && !q2Running) {
            sortQueue(q2);
        }

        if (!q3.empty() && currentTime >= q3.front().arrivalTime && !q3Running) {
            sortQueue(q3);
        }

        currentTime++;
    }

    displayOutput(all_info, num);

    for (int i = 0; i < 3; i++) {
        delete[] all_info[i];
    }
}

void CPUscedulingStart()
{
     multiLevel();
}