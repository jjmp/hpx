////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2012 Andrew Kemp
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

// Naive SMP version implemented with futures.

#include <hpx/hpx_init.hpp>
#include <hpx/include/actions.hpp>
#include <hpx/include/util.hpp>
#include <hpx/include/lcos.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <boost/algorithm/string.hpp>


std::vector<std::string> words;

std::string search(int start, int end, std::string &word);

HPX_PLAIN_ACTION(search, search_action);

std::string search(int start, int end, std::string &word)
{
    //highest value is 'z' at 122
    //lowest value is 'a' at 97
    //start is where our word check is located.
    //end is where the end if this thread's search is.
    int mid = (start+end);
    std::string check = words[mid/2];
    //first we check if there is no definitive match
    if (start == end && word != check)
    {
        //just a quick check, because the list is not perfectly symmetrical
        {
            int pos = mid/2;
            
            int size;
            //if our value is lower than start, we disregard it.
            if (word.length() >= check.length())
                size = check.length();
            else
                size = word.length();
            std::string part;
            bool sub = true;
            for (int i = 0; i < size; i++)
            {
                char check_char = tolower(check[i]);
                char word_char = word[i];
                if (word_char != check_char)
                {
                    if (word_char >=check_char)
                        sub = false;
                    break;
                }
                else
                    part.push_back(word_char);
            }
            while(check != word)
            {
                check = words[pos];
                if (sub)
                    pos--;
                else
                    pos++;
                if (check.find(part) == std::string::npos)
                    return word + " was not found in this dictionary, " + check + " was the closest match.\n";
            }
            return "";
        }
    }
    int size;
    //if our value is lower than start, we disregard it.
    if (word.length() >= check.length())
        size = check.length();
    else
        size = word.length();
    for (int i = 0; i < size; i++)
    {
        char check_char = tolower(check[i]);
        char word_char = word[i];
        if (check_char != word_char)
        {
            if (word_char > check_char)
                return search((mid+1)/2, end, word);
            else
                return search(start, (mid-1)/2, word);
        }
    }
    if (check.length() == word.length())
        return "";
    else
        return search((start+end+1)/2, end, word);
}
int hpx_main()
{
    
    {
        using namespace std;
        ifstream fin;
        string path = __FILE__;
        string wordlist_path;
        string remove = "spell_check_file.cpp";
        for (int i = 0; i < path.length() - remove.length(); i++)
        {
            wordlist_path.push_back(path[i]);
            if (path[i] == '\\')
            {
                wordlist_path.push_back(path[i]);
            }
        }
        //list of American English words in alphabetical order. Provided by Kevin at http://wordlist.sourceforge.net/
        
        string word;
        cout << "Loading file: example_text.txt\n";
        hpx::util::high_resolution_timer t;
        
        int fileLines = 0;;
        std::string example_path = wordlist_path + "example_text.txt";
        fin.open(example_path);
        if(fin.is_open())
        {
            string temp;
            while (fin.good())
            {
                fin >> temp;
                for each(char c in temp)
                word.push_back(c);
                word.push_back(' ');
                fileLines++;
            }
            cout << fileLines << " lines loaded in " << t.elapsed() << "s.\n";
        }
        else
        {
            cout << "Error: Unable to open file.\n";
            return hpx::finalize();
        }
        vector<bool> contraction;
        vector<string> strs;
        {
            vector<string> temp;
            boost::split(temp, word, boost::is_any_of("\n\t -"));
            for (int i = 0; i < temp.size(); i++)
            {
                bool isContraction = false;
                string holder;
                for (int j = 0; j < temp[i].size(); j++)
                {
                    //a size check to avoid errors
                    if (temp[i].size() - j - 1 == 2)
                    {
                    //if this is a contraction, ignore the rest of it...
                        if (temp[i][j+1] == '\'' && temp[i][j] == 'n' && temp[i][j+2] == 't')
                        {
                            //but label this as a contraction
                            isContraction = true;
                            break;
                        }
                    }
                    //remove any garbage characters
                    if (toupper(temp[i][j]) >= 'A' && toupper(temp[i][j]) <= 'Z')
                        holder.push_back(tolower(temp[i][j]));
                }
                if (holder.size() > 0)
                {
                    contraction.push_back(isContraction);
                    strs.push_back(holder);
                }
            }
        }
        fin.close();
        wordlist_path = wordlist_path + "5desk.txt";
        fin.open(wordlist_path);
        int wordcount = 0;
        cout << "Reading dictionary file to memory...\n";
        t.restart();
        if(fin.is_open())
        {
            string temp;
            while (fin.good())
            {
                getline(fin, temp);
                for (int i = 0; i < temp.length(); i++)
                temp[i] = tolower(temp[i]);
                words.push_back(temp);
                wordcount++;
            }
            cout << wordcount << " words loaded in " << t.elapsed() << "s.\n";
        }
        else
        {
            cout << "Error: Unable to open file.\n";
            return hpx::finalize();
        }
        fin.close();
        t.restart();
        {
            using hpx::lcos::future;
            using hpx::async;
            using hpx::wait_all;
            vector<search_action> sAct;//[sizeX * sizeY];
            vector<future<string>> wordRun;
            wordRun.reserve(strs.size());
            for each(string single in strs)
            {
                int start = 0;
                hpx::naming::id_type const locality_id = hpx::find_here();
                search_action temp;
                wordRun.push_back(async(temp, locality_id, start, wordcount, single));
                sAct.push_back(temp);
                //cout << search(0, wordcount, single) << endl;
            }
            wait_all(wordRun);
            cout << "Search completed in " << t.elapsed() << "s.\n";
            for (int i = 0; i < strs.size(); i++)
            {
                string get = wordRun[i].get();
                if (get.size() > 0)
                {
                    cout << "Word number " << i + 1 << ":\n";
                    if (contraction[i])
                        cout << "Note: This word seems to be a contraction.\nThe last two letters have been ignored.\n";
                    cout << get;
                }
            }
        }
    }
    return hpx::finalize(); // Handles HPX shutdown
}
int main()
{
    int code = hpx::init();
    std::cout << std::endl;
    system("Pause");
    return code;
}