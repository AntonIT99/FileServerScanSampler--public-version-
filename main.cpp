#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <list>
#include <cstdlib>
#include <ctime>
#include "timer.h"

using namespace std;

unsigned int desiredNumFolders = 50; ///desired amount of folders for the sample
unsigned int desiredNumFiles = 1000; ///desired approximated amount of files for the sample
float precision = 0.1; ///allowed gap relative to the desiredNumFiles value : the smaller, the better is the precision

string ALLOWED_EXTENSIONS[] = {"docx", "docm", "dot", "dotm", "dotx", "xls", "xlt", "xlsx", "xltx", "xltm", "xlsm", "xlsb", "ppt", "pps", "pot", "pptx", "ppsx", "pptm", "ppsm", "potx", "potm", "pdf", "txt", "xml", "csv"};

//convert an integer to a string
string intToString(int val)
{
    stringstream ss;
    ss << val;
    return ss.str();
}

class Folder
{
  public:
    string name;
    string fullPath;
    int numberOfFiles;

    Folder(string n, string path)
    {
        name = n;
        fullPath = path;
        numberOfFiles = 0;
    }

    ~Folder() {};

    string toString()
    {
        return "name: " + name + " | files: " + intToString(numberOfFiles);
    }
};

//split a string in a vector of strings using a delimiter
void split(const string &str, const char delim, vector<string> &out)
{
	stringstream ss(str);
	string s;

	while (getline(ss, s, delim))
    {
		out.push_back(s);
	}
}

//put a string to lowercase
void toLowercase(basic_string<char> &s)
{
    for (basic_string<char>::iterator p = s.begin(); p != s.end(); ++p)
    {
      *p = tolower(*p);
    }
}

//returns true if the extension is included in the list allowedExt, returns false otherwise
bool checkExtension(string fileExt, string allowedExt[], int allowedExtNum)
{
    bool test = false;
    for (int i=0; i<allowedExtNum; i++)
    {
        if (allowedExt[i].compare(fileExt) == 0)
        {
            test = true;
        }
    }
    return test;
}

//returns the amount of files contained in all folders of the list
int fileAmount(vector<Folder> f)
{
    int files = 0;
    for (unsigned i=0; i<f.size(); i++)
    {
        files += f[i].numberOfFiles;
    }
    return files;
}

//returns a random integer between min and max values
int randomInt(int minVal, int maxVal)
{
    srand(time(NULL));
    return rand()%(maxVal-minVal + 1) + minVal;
}

//check if the value a is included in [b - c*b ; b + c*b]
//for example enter c = 0.1 if you want a to be a least an approximation of b + or - 10% of b
//returns 0 if a is included in the interval
//returns 1 if a is superior to b + c*b
//returns -1 if a is inferior to b - c*b
int isApproximation(int a, int b, float precision)
{
    int test = 0;
    if ((float)(a) < (1-precision)*(float)(b))
    {
        test = -1;
    }
    if ((float)(a) > (1+precision)*(float)(b))
    {
        test = 1;
    }
    return test;
}

//returns the index of the folder of the list containing the smallest amount of files provided the list isn't empty
int lightestFolder(vector<Folder> f)
{
    int index = 0;
    for (unsigned i=1; i<f.size(); i++)
    {
        if (f[i].numberOfFiles < f[index].numberOfFiles)
        {
            index = i;
        }
    }
    return index;
}

//returns the index of the folder of the list containing the biggest amount of files provided the list isn't empty
int heaviestFolder(vector<Folder> f)
{
    int index = 0;
    for (unsigned i=1; i<f.size(); i++)
    {
        if (f[i].numberOfFiles > f[index].numberOfFiles)
        {
            index = i;
        }
    }
    return index;
}

//move all subdirectories of folder f from list 1 to list 2
void moveSubDir(Folder f, vector<Folder> &list1, vector<Folder> &list2)
{
    for (unsigned i=0; i<list1.size(); i++)
    {
        size_t test = list1[i].fullPath.find(f.fullPath);
        if (test !=string::npos && list1[i].fullPath.compare(f.fullPath) != 0)
        {
            list2.push_back(list1[i]);
            list1.erase(list1.begin()+i);
        }
    }
}

//check for subdirectories and duplicate folders
void check(vector<Folder> &selectionList, vector<Folder> &subDirBlackList)
{
    cout << "checking sample..." << endl;
    for (unsigned i=0; i<selectionList.size(); i++)
    {
        for (unsigned j=0; j<selectionList.size(); j++)
        {
            size_t test = selectionList[j].fullPath.find(selectionList[i].fullPath);
            if (selectionList[i].fullPath.compare(selectionList[j].fullPath) == 0 && i != j) //if twice the same directory
            {
                cout << selectionList[j].name << "   [removed] | reason: duplicate of " << selectionList[i].name << endl;
                selectionList.erase(selectionList.begin()+j);

            }
            if (test !=string::npos && selectionList[i].fullPath.compare(selectionList[j].fullPath) != 0) //if subdirectories
            {
                cout << selectionList[j].name << "   [removed] | reason: subdirectory of " << selectionList[i].name << endl;
                subDirBlackList.push_back(selectionList[j]);
                selectionList.erase(selectionList.begin()+j);
            }
        }
    }
    cout << "checking done" << endl;
}


int main(int argc , char *argv[])
{
    bool enableTimer = false;
    Timer timer;

    if (argc > 1)
    {
        for (int i = 0; i<argc; i++)
        {
            string stringArg(argv[i]);
            if (stringArg.compare("-t") == 0)
            {
                enableTimer = true;
            }
        }
    }

    int totalNumberOfFiles = 0;
    string line;
    ifstream inFile;
    ofstream outFile;
    vector<Folder> folderList; //list representing all the folders
    vector<Folder> selectedFolderList; //list representing the selected folders for the sample
    vector<Folder> subDirBlackList; //when a folder is moved from folderList to selectedFolderList, all its subdirectories go there

    inFile.open("FILESERVER-FULL-SCAN.txt");
    system("chcp 1252"); //allow printing of special characters for french words for the windows console

    if (!inFile)
    {
        cout << "Unable to open file FILESERVER-FULL-SCAN.txt" << endl;
        exit(1);
    }

    while (getline(inFile, line))
    {
        size_t testLine = line.find("\\\\");
        if (testLine !=string::npos) //only consider lines that contain "\\", skip other lines
        {
            vector<string> linePart;
            vector<string> fileName;
            string fileExt;
            string path = "\\\\";

            split(line, '\\', linePart);

            split(linePart[linePart.size()-1], '.', fileName);

            fileExt = fileName[fileName.size()-1];
            toLowercase(fileExt);

            if (checkExtension(fileExt, ALLOWED_EXTENSIONS, sizeof(ALLOWED_EXTENSIONS)/sizeof(*ALLOWED_EXTENSIONS)))
            {
                totalNumberOfFiles++;
                for (unsigned i=2; i<linePart.size()-1; i++)
                {
                    path += linePart[i] + "\\";
                    bool isFolderAlreadyRegistered = false;
                    for (unsigned j=0; j<folderList.size(); j++)
                    {
                        if (folderList[j].fullPath.compare(path) == 0)
                        {
                            isFolderAlreadyRegistered = true;
                            folderList[j].numberOfFiles++;
                        }
                    }
                    if (!isFolderAlreadyRegistered)
                    {
                        Folder f(linePart[i], path);
                        f.numberOfFiles++;
                        folderList.push_back(f);
                    }

                }
            }
        }
    }

    inFile.close();

    cout << endl << "Amount of files detected: " << totalNumberOfFiles << endl;
    cout << "Amount of folders containing detected files: " << folderList.size() << endl << endl;

    int percent;
    cout << endl << "Amount of folders for the sample: ";
    cin >> desiredNumFolders;
    cout << endl << "Approximate amount of files for the sample: ";
    cin >> desiredNumFiles;
    cout << endl << "Maximal gap allowed, in %, relative to the amount of files: ";
    cin >> percent;
    precision = (float)(percent)/100.000;

    ///selection process

    if (enableTimer) timer.start();

    bool stop = false;

    while ((selectedFolderList.size() < desiredNumFolders || isApproximation(fileAmount(selectedFolderList), desiredNumFiles, precision) != 0) && !stop)
      {
        if (selectedFolderList.size() < desiredNumFolders) //adding folders to the selection to reach the desired amount
        {
            int randomIndex = randomInt(0, folderList.size()-1);
            selectedFolderList.push_back(folderList[randomIndex]);
            folderList.erase(folderList.begin()+randomIndex);
            moveSubDir(selectedFolderList[selectedFolderList.size()-1], folderList, subDirBlackList);
            moveSubDir(selectedFolderList[selectedFolderList.size()-1], selectedFolderList, subDirBlackList);
            cout << selectedFolderList[selectedFolderList.size()-1].name << "   [added]" << endl;
            cout << "files : " << fileAmount(selectedFolderList) << endl;
        }
        else if (isApproximation(fileAmount(selectedFolderList), desiredNumFiles, precision) < 0) //not enough files : swapping with folders containing more files
        {
            cout << "files : " << fileAmount(selectedFolderList) << endl;
            bool swappingDone = false;
            int indexToSwap = lightestFolder(selectedFolderList);

            for (unsigned i=randomInt(0, folderList.size()-1); !swappingDone; i = randomInt(0, folderList.size()-1))
            {
                if (folderList[i].numberOfFiles > selectedFolderList[indexToSwap].numberOfFiles)
                {
                    Folder swapFolder = selectedFolderList[indexToSwap];
                    selectedFolderList[indexToSwap] = folderList[i];
                    folderList[i] = swapFolder;
                    moveSubDir(selectedFolderList[indexToSwap], folderList, subDirBlackList);
                    moveSubDir(selectedFolderList[indexToSwap], selectedFolderList, subDirBlackList);

                    cout << folderList[i].name << "   [added]" << endl;

                    if (selectedFolderList.size() < desiredNumFolders)
                    {
                        selectedFolderList.push_back(swapFolder);
                        folderList.erase(folderList.begin()+i);

                    } else {
                        moveSubDir(swapFolder, subDirBlackList, folderList);
                        cout << selectedFolderList[indexToSwap].name << "   [removed]" << endl;
                    }
                    swappingDone = true;
                }
            }
            if (!swappingDone)
            {
                cout << "Not enough files available to reach the desired amount, program aborting... Restart and choose a smaller number of files or change the desired precision" << endl;
                stop = true;
            }
        }
        else if (isApproximation(fileAmount(selectedFolderList), desiredNumFiles, precision) > 0) //too much files : swapping with folders containing less files
        {
            cout << "files: " << fileAmount(selectedFolderList) << endl;
            bool swappingDone = false;
            int indexToSwap = heaviestFolder(selectedFolderList);

            int tries = 0;

            for (unsigned i=randomInt(0, folderList.size()-1); !swappingDone && tries < 100; i = randomInt(0, folderList.size()-1))
            {
                if (isApproximation(selectedFolderList[indexToSwap].numberOfFiles - folderList[i].numberOfFiles, fileAmount(selectedFolderList) - desiredNumFiles, 0.5) == 0)
                {
                    Folder swapFolder = selectedFolderList[indexToSwap];
                    selectedFolderList[indexToSwap] = folderList[i];
                    folderList[i] = swapFolder;
                    moveSubDir(selectedFolderList[indexToSwap], folderList, subDirBlackList);
                    moveSubDir(selectedFolderList[indexToSwap], selectedFolderList, subDirBlackList);
                    moveSubDir(swapFolder, subDirBlackList, folderList);
                    swappingDone = true;
                    cout << folderList[i].name << "   [added]" << endl;
                    cout << selectedFolderList[indexToSwap].name << "   [removed]" << endl;
                }
                tries++;
            }

            indexToSwap = heaviestFolder(selectedFolderList);
            cout << "files : " << fileAmount(selectedFolderList) << endl;

            for (unsigned i=randomInt(0, folderList.size()-1); !swappingDone && tries < 10000; i = randomInt(0, folderList.size()-1))
            {
                if (folderList[i].numberOfFiles < selectedFolderList[indexToSwap].numberOfFiles)
                {
                    Folder swapFolder = selectedFolderList[indexToSwap];
                    selectedFolderList[indexToSwap] = folderList[i];
                    folderList[i] = swapFolder;
                    moveSubDir(selectedFolderList[indexToSwap], folderList, subDirBlackList);
                    moveSubDir(selectedFolderList[indexToSwap], selectedFolderList, subDirBlackList);
                    moveSubDir(swapFolder, subDirBlackList, folderList);
                    swappingDone = true;
                    cout << folderList[i].name << "   [added]" << endl;
                    cout << selectedFolderList[indexToSwap].name << "   [removed]" << endl;
                }
            }

            if (!swappingDone) //still too much files, suppressing folders of the list
            {
                while (isApproximation(fileAmount(selectedFolderList), desiredNumFiles, precision) > 0)
                {
                    int indexToDelete = heaviestFolder(selectedFolderList);
                    folderList.push_back(selectedFolderList[indexToDelete]);
                    moveSubDir(selectedFolderList[indexToDelete], subDirBlackList, folderList);
                    selectedFolderList.erase(selectedFolderList.begin()+indexToDelete);
                    desiredNumFolders--;
                    cout << folderList[folderList.size()-1].name << "   [removed]" << endl;
                }
            }
        }
        if (isApproximation(fileAmount(selectedFolderList), desiredNumFiles, precision) == 0)
        {
            check(selectedFolderList, subDirBlackList);
        }
    }

    if(!stop)
    {
        cout << endl << "Selection of " << selectedFolderList.size() << " folders containing " << fileAmount(selectedFolderList) << " files." << endl << endl;

        outFile.open("SCAN-LIST.txt");
        if (!outFile)
        {
            cout << "Unable to create or to read file SCAN-LIST.txt" << endl;
            exit(0);
        }

        for (unsigned i=0; i<selectedFolderList.size(); i++)
        {
            cout << selectedFolderList[i].toString() << endl;
            outFile << selectedFolderList[i].fullPath <<endl;

        }
        outFile.close();
    }

    if (enableTimer)
    {
        timer.stop();
        cout << endl << endl << "Seconds: " << timer.elapsedSeconds() << endl;
        cout << "Milliseconds: " << timer.elapsedMilliseconds() << endl;
    }

    cout << endl << "Press a key to exit" << endl;
    getchar();

    return 0;
}
