#include <iostream>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <ftw.h>
#include <fnmatch.h>
#include <fstream>
#include <errno.h>
#include <iostream>

using namespace std;
string backup = "";
string target = "";
bool targetdir, backupdir;
int modifiednum = 0;
bool modified;

static int callbackdir(const char *fpath, const struct stat *sb, int typeflag)
{
    if (typeflag == FTW_D)
    {
        if (string(fpath).length() >= backup.length())
        {
            string path = string(fpath);
            path = path.substr(backup.length(),path.length()-backup.length());
            if (mkdir((target+path).c_str(),S_IRWXU | S_IRGRP |  S_IXGRP | S_IROTH | S_IXOTH) == -1)
            {
                if (errno == EEXIST)
                {
                    cout<<"Folder "<<path<<" already exists.\n";
                }
                else
                {
                    cout<<"Cannot create folder "<<target+path<<" errno "<<errno<<endl;;
                }
            }

        }
    }
    return 0;
}

static int callbackfile(const char *fpath, const struct stat *sb, int typeflag)
{
    if (typeflag == FTW_F)
    {


        int fileday, filemonth, fileyear;
        int modday, modmonth, modyear;
        struct stat attrib;
        stat(fpath,&attrib);
        char day[3];
        strftime(day,3,"%d",localtime(&(attrib.st_mtime)));
        fileday = atoi(day);
        char month[3];
        strftime(month,3,"%m",localtime(&(attrib.st_mtime)));
        filemonth = atoi(month);
        char year[5];
        strftime(year,5,"%y",localtime(&(attrib.st_mtime)));
        fileyear = atoi(year);
        string path = string(fpath);
        ifstream src(path.c_str(),ios::binary);
        path = path.substr(backup.length(),path.length()-backup.length());
        ofstream dest((target+path).c_str(),ios::binary);

        time_t timev = time(0);
        struct tm tstruct;
        tstruct = *localtime(&timev);
        char cday[3];
        strftime(cday,3,"%d",&tstruct);
        modday = atoi(cday);
        char cmonth[3];
        strftime(cmonth,3,"%m",&tstruct);
        modmonth = atoi(cmonth);
        char cyear[5];
        strftime(cyear,5,"%y",&tstruct);
        modyear = atoi(cyear);

        int monthlengths[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
        modday -= modifiednum;
        if (modday < 1)
        {
            modmonth -=1;
            modday = monthlengths[modmonth] - (1-modday);
        }
        if (modmonth < 1)
        {
            modyear -= 1;
            modmonth = 12;
        }

        bool move = false;
        if (fileyear > modyear)
        {
            cout<<"Year: "<<fileyear<<" > "<<modyear<<endl;
            move = true;
        }
        else if (fileyear == modyear)
        {
            if (filemonth > modmonth)
            {
                move = true;
                cout<<"Month: "<<filemonth<<" > "<<modmonth<<endl;
            }
            else if (filemonth == modmonth)
            {
                if (fileday >= modday)
                {
                    cout<<"Day: "<<fileday<<" >= "<<modday<<endl;
                    move = true;
                }
            }
        }

        if (move)
        {
            cout<<"File dated "<<fileday<<"/"<<filemonth<<"/"<<fileyear<<" backed up at "<<target+path<<endl;
            dest<<src.rdbuf();
        }

        src.close();
        dest.close();
        //cout<<"File created "+target+path;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc > 0)
    {
        for (int i=1;i<argc;i+=2)
        {
            if (string(argv[i]) == "-b")
            {
                if (argc > i)
                {
                    backup = argv[i+1];
                    backupdir = true;
                    cout<<"Processing folder for backup "<<argv[i+1]<<endl;
                }
                else
                {
                    cout<<"Failed to process backup folder, no path given.\n";
                }
            }
            else if (string(argv[i]) == "-t")
            {
                if (argc > i)
                {
                    target = argv[i+1];
                    targetdir = true;
                    cout<<"Processing target folder "<<argv[i+1]<<endl;
                }
                else
                {
                    cout<<"Failed to process target folder, no path given.\n";
                }
            }
            else if(string(argv[i]) == "-m")
            {
                if (argc > i)
                {
                    modifiednum = atoi(string(argv[i+1]).c_str());
                    modified = true;
                }
            }
            else
            {
                cout<<"Unknown parameter "<<argv[i]<<endl;
            }
        }
    }

    if (!targetdir)
    {
        target = "/home/chris/Target/";
    }
    if (!backupdir)
    {
        backup = "/home/chris/Backup/";
    }
    if (!modified)
    {
        modifiednum = 1;
    }
    if (backup.substr(backup.length()-1,backup.length()) != "/")
    {
        backup+="/";
    }
    if (target.substr(target.length()-1,target.length()) != "/")
    {
        target+="/";
    }

    if (mkdir((target).c_str(),S_IRWXU | S_IRGRP |  S_IXGRP | S_IROTH | S_IXOTH) == -1)
    {
        if (errno == EEXIST)
        {
            cout<<"Folder "<<target<<" already exists.\n";
        }
        else
        {
            cout<<"Cannot create folder "<<target<<" errno "<<errno<<endl;;
        }
    }
    char today[100];
    time_t now = time(0);
    struct tm nowstruct;
    nowstruct = *localtime(&now);
    strftime(today,sizeof(today),"%y-%m-%d-backup",&nowstruct);
    target+=string(today)+"/";
    if (mkdir((target).c_str(),S_IRWXU | S_IRGRP |  S_IXGRP | S_IROTH | S_IXOTH) == -1)
    {
        if (errno == EEXIST)
        {
            cout<<"Folder "<<target<<" already exists.\n";
        }
        else
        {
            cout<<"Cannot create folder "<<target<<" errno "<<errno<<endl;;
        }
    }

    ftw(backup.c_str(),callbackdir,16);
    ftw(backup.c_str(),callbackfile,16);



    return 0;
}
