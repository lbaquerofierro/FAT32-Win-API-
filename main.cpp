#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <bitset> 
#include <algorithm>

using namespace std;

#pragma pack(1)
typedef struct _BIOS_PARAM_BLOCK
{
	BYTE jump_instruction[3];
	BYTE oem[8];
	WORD bytes_Sector;					//Required//
	BYTE sec_Cluster;					//Required//
	WORD size_Sector_Reserved;			
	BYTE fatCount;						//Required//
	WORD Max_Root_Entry;
	WORD Total_Sector_FS; 
	BYTE media_type;
	WORD fat_sectors;
	WORD sec_per_track; 
	WORD num_head;
	DWORD num_before_part; 
	DWORD no_Sector_FS32;				 //Required//
	DWORD FATSz32;
	WORD ExtFlags;
	WORD FsVer;
	DWORD RootClus;
	BYTE rest[464];
} BPB;
#pragma pack()

#pragma pack(2)
typedef struct dir{
	BYTE fName[8];
	BYTE ext[3];
	BYTE rest[7];
	WORD date;
	BYTE rest2[6];
	WORD start_clus_no;
	DWORD fSize;
} DIR;
#pragma pack()

//This function prints the boot sector information
void PrintDiskInformation(BPB _bpb);

//This function prints the file information
void PrintFileInformation(DIR _fpb);

//This function parses the date
void FindDate(WORD _fpb);


int main()
{

	cout << "Lina Baquero" << endl;
	cout << "FAT32 Lab - CSCI 443" << endl << endl; 


	BYTE bBootSector[512];				//Store boot sector		
	DWORD dwBytesRead(0);				//Number of read byes
	BPB _bpb;							//Boot sector data structure variable
	string drive;						//User letter input
	HANDLE hDrive = NULL;				//Handle to disk

	/*############    Read Drive Letter     #############*/
	cout << "Enter thumb drive letter: ";
	cin >> drive;
	drive = "\\\\.\\" + drive + ":";

	/*############    Open Drive     #############*/
	hDrive = CreateFileA(drive.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	/*#########    Read Boot Sector     ##########*/
	if (hDrive != NULL)
	{
		// If drive exists and opened correctly go to else
		if (!ReadFile(hDrive, bBootSector, 512, &dwBytesRead, NULL))
		{
			printf("\nError in reading the drive/it does not exists\n");
		}
		else
		{
			//Copy buffer to data structure for boot sector
			memcpy(&_bpb, bBootSector, 512);

			//Print drive information
			PrintDiskInformation(_bpb);

			/*#########    Read Root Directory     ##########*/

			//Variable that wil be used to parse the root directory 
			BYTE fileInfo[512];			//Store unit (sector) 
			DIR _fpb;					//Directory struct data type
			int rootDirStart;			//Calculated start of root directory
			DWORD dwCurrentPosition;	//Store new current position


			//Calculate start of root directory
			rootDirStart = _bpb.size_Sector_Reserved + (_bpb.fatCount * _bpb.FATSz32);

			//Move pointer to root directory entry
			SetFilePointer(
				hDrive,
				((rootDirStart - 1) * 512),
				NULL,
				FILE_CURRENT);

			//Read root directory (minimum of 1 cluster)
			if (!ReadFile(hDrive, fileInfo, 512, &dwBytesRead, NULL))
			{
				printf("\nError in reading root directory\n");
			}
			else
			{
				//Local variables
				string fileName; //User input

				cout << "\nPlease enter a file name in the format 'testfile.ext': ";
				cin >> fileName;

				if (fileName.size() > 12 || fileName.size() < 5){
					cout << "\nFile name is invalid\n";
					return 0;
				}
				else {
					//Local Variables
					int dotPos;			//dot position in user input
					string s;			//parsed user input (name of file)
					string ex;			//parsed user input (extension name)
					string t;			//parsed drive file name
					string e;			//parsed drive extension name
					bool testVar;		//To keel parsing through sector 
					BYTE fInfo[32];		//To store file struct
					int start;			//specific directory start
					int finish;			//specific directory end
					int k;				//To copy sector array to file array
					int aCluster;		//Increment if file not find in sector
					

					dotPos = fileName.find('.', 0); 
					s = fileName.substr(0, dotPos);
					ex = fileName.substr(dotPos + 1, 3);

					//Convert to upper case to compare agains ascii from hex dump
					transform(s.begin(), s.end(), s.begin(), toupper);
					transform(ex.begin(), ex.end(), ex.begin(), toupper);
					
					//Initialize some required variables
					start = 0;
					finish = 32;
					k = 0;
					aCluster = 0;
					testVar = TRUE; 

					//While the stored file in not the one being searched 
					while (testVar){
						k = 0;
						for (int i = start; i < finish; i++){
							fInfo[k] = fileInfo[i];
							k++;
						}

						memcpy(&_fpb, fInfo, 32);

						//Convert drive name and extension to required size 
						t = string(reinterpret_cast<char *>(_fpb.fName), s.size());
						e = string(reinterpret_cast<char *>(_fpb.ext), 3);

						//Check if enterd name matches drive name after conversion
						if (s == t && ex == e){
							//Print file info
							PrintFileInformation(_fpb);
							testVar = FALSE;
						}
						else{ //Go to next directory entry
							start += 32;
							finish += 32;
						}
						//If not found in first sector go to next sector
						if (finish == 512){
							ReadFile(hDrive, fileInfo, 512, &dwBytesRead, NULL); //Need validation!
							start = 0;
							finish = 32;
							continue; 

							//cout << endl << "File was not encountered in first sector << endl; 
							//return 0; 
						}

						//This would add functionality to read more than one secor
						/*This can be configured as shown in the comment to be 2 clusters (entire root directory)
						Comment line 182 and uncomment 187 to accomplish this*/
						aCluster++;
						if (aCluster >= (_bpb.bytes_Sector * _bpb.sec_Cluster)){ //8 * 512 = 4096; //Root cluster = 2 --> *2?
							cout << "\nThe file does not exist\n" << endl;
							break;
						}
					}
				}
			}
		}
		CloseHandle(hDrive);
	}
	system("pause");
	return 0;
}

void PrintDiskInformation(BPB _bpb)
{
	cout << "\nDRIVE INFORMATION:\n";
	printf("===========================\n");
	printf("Bytes per sector: %d\n", _bpb.bytes_Sector);
	printf("Sector per cluster: %d\n", _bpb.sec_Cluster);
	printf("Number of FATs(File Allocation Table): %d\n", _bpb.fatCount);
	printf("Number of logical sector fields used by FAT32: %d\n", _bpb.no_Sector_FS32);
}

void PrintFileInformation(DIR _fpb){
	cout << "\nFILE INFORMATION:\n";
	cout << "============================\n";
	printf("Starting cluster number: %d\n", _fpb.start_clus_no);
	printf("File size: %d bytes\n", _fpb.fSize);
	printf("Last day accessed: ");
	FindDate(_fpb.date);
	printf("\n");
}

void FindDate(WORD _fpb){
	bitset <16> bin(_fpb);
	string binNum = bin.to_string();

	string yearBin, monthBin, dayBin;
	bitset <7> year(binNum.substr(0, 7));
	bitset <4> month(binNum.substr(7, 4));
	bitset <5> day(binNum.substr(11, 5));

	int y, m, d;
	y = year.to_ulong();
	m = month.to_ulong();
	d = day.to_ulong();

	cout << (y + 1980) << "/" << m << "/" << d << endl;
}