/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   main.cpp
 * Author: Jörn Roddelkopf
 * Version: 1.0 20.09.2016
 * Purpose: Create a Blocklist for Bind9 DNS Service
 */

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>

/**
 * Replace a Substring with another String inside a String.
 * @param str String to be searched in
 * @param from String to seach for
 * @param to String to replace with
 * @return True if String was found and replaced. False if not found.
 */
bool replace(std::string& str, const std::string& from, const std::string& to) 
{
    size_t start_pos = str.find(from);
    
    if(start_pos == std::string::npos)
        return false;
    
    while((start_pos = str.find(from)) != std::string::npos)
        str.replace(start_pos, from.length(), to);
    
    return true;
}

/**
 * Reads a File Line by Line and returns the Content as String Vector.
 * @param file_name Path to the File to read
 * @return 
 */
std::vector<std::string> read_file(std::string& file_name)
{
    std::ifstream file(file_name.c_str());
    std::string str;
    std::vector<std::string> entry_list;
    
    while(std::getline(file, str))
    {
        entry_list.push_back(str);
    }  
    
    file.close();
    
    return entry_list;
}

/**
 * Checks if the given List contains duplicates and removes them.
 * @param entry_list List of Strings to remove duplicates from
 * @return String List without Duplicates
 */
std::vector<std::string> remove_duplicates(std::vector<std::string>& entry_list)
{
    std::vector<std::string> result_list;
    bool is_found = false;
    int position = 0;
    
    for(unsigned long int i = 0; i < entry_list.size(); i++)
    {
        is_found = false;

        while(entry_list.at(i).length() > 0 && position < entry_list.at(i).length() && entry_list.at(i).at(position) == ' ')
            entry_list.at(i) = entry_list.at(i).substr(++position);
        
        position = entry_list.at(i).length() - 1;

        while(entry_list.at(i).length() > 0 && position < entry_list.at(i).length() && (entry_list.at(i).at(position) == ' ' || entry_list.at(i).at(position) == '\n' || entry_list.at(i).at(position) == '\r'))
        {
            entry_list.at(i) = entry_list.at(i).substr(0, position);
            position--;
        }

        if(entry_list.at(i).length() == 0)
            continue;
        
        for(unsigned long int x = 0; x < result_list.size(); x++)
        {
            if(entry_list.at(i).compare(result_list.at(x)) == 0)
            {
                is_found = true;
                std::cout << "Duplicated Entry skipped: " << entry_list.at(i) << std::endl;
                break;
            }
        }
        
        if(!is_found)
            result_list.push_back(entry_list.at(i));
    }
    
    return result_list;
}

/**
 * Write the List of Domains to a proper Bind9 Domain Zone List File.
 * @param path_list Path to the Zone List File
 * @param path_zone Path to the Zone Definition File
 * @param entry_list List containing the Domains
 */
void write_blocklist(std::string& path_list, std::string& path_zone, std::vector<std::string>& entry_list)
{
    std::ofstream file;
    file.open(path_list.c_str());

    for(unsigned long int i = 0; i < entry_list.size(); i++)
    {
        // zone "101com.com" { type master; notify no; file "/etc/bind/null.zone.file"; };
        file << "zone \"" + entry_list.at(i) + "\" { type master; notify no; file \"" << path_zone << "\"; allow-update { none; }; };" << "\n";
    }
    
    file.flush();
    file.close();
}

/**
 * Write a proper Bind9 Zone Definition File.
 * @param path_zone Path to the Zone Definition File
 * @param direct_to IP Address to resolv the Domain to
 */
void write_zonefile(std::string& path_zone, std::string& direct_to)
{
    std::string content = 
    "$TTL	86400\n"
    "@	IN	SOA	adblocker.bind.local.	ads.bind.local. ("
			"2016012218 " // Add Current Date as ID
			"28800 "
			"7200 "
			"864000 "
			"86400 )\n"
    "@	IN	NS	adblocker.bind.local.\n"
    "@	IN	A	" + direct_to + "\n"
    "*	IN	A	" + direct_to;

    std::ofstream file;
    file.open(path_zone.c_str());
    
    file << content;
    
    file.flush();
    file.close();
}

/**
 * Write Include Directive to the Bind Configuration File. Checks if the
 * Include is already present.
 * @param bind_cfg Path to the Bind Configuration File
 * @param zone_file Path to the Zone List File
 */
void edit_bind_cfg(std::string bind_cfg, std::string zone_file)
{
    std::vector<std::string> f_content = read_file(bind_cfg);
    
    for(unsigned long int i = 0; i < f_content.size(); i++)
    {
        if(f_content.at(i).find(zone_file) != std::string::npos)
        {
            return;
        }
    }
    
    std::ofstream file;
    file.open(bind_cfg.c_str());

    file << "include \"" << zone_file << "\";" << std::endl;
    
    for(unsigned long int i = 0; i < f_content.size(); i++)
    {
        file << f_content.at(i) << "\n";
    }
    
    file.flush();
    file.close();
}

/**
 * Splits a String with the given delimiter and removes the delimiter
 * @param text The Text to Split
 * @param delimit The Delimiter to use for splitting
 * @return List of Strings
 */
std::vector<std::string> split(std::string text, std::string& delimit)
{
    std::vector<std::string> seglist;
    size_t position = text.find(delimit);
    
    if(position == std::string::npos)
    {
        seglist.push_back(text);
        return seglist;
    }
    
    while((position = text.find(delimit)) != std::string::npos)
    {
        seglist.push_back(text.substr(0, position));
        text = text.substr(position + delimit.length());
    }

    if(text.length() > 0)
        seglist.push_back(text);
    
    return seglist;
}

/**
 * Copy a File.
 * @param source Path to the File that should be copied
 * @param target Path to copy the File to
 */
void copy_file(std::string& source, std::string& target)
{
    std::vector<std::string> s_file = read_file(source);
    
    std::ofstream file;
    file.open(target.c_str());
    
    for(unsigned long int i = 0; i < s_file.size(); i++)
        file << s_file.at(i) << "\n";
    
    file.flush();
    file.close();
}

int main(int argc, char** argv) 
{
    std::cout << "BIND Blocklist Generator v.1.0\n\n";
    
    bool    show_hints = false,
            redirect = false,
            reload_zones = false,
            no_bind_cfg = false,
            custom_zone = false,
            link_zone = false;
    std::string redir_ip = "127.0.0.1";
    std::string zone_file = "null.zone";
    std::string str_argv = std::string(argv[0]);
    
    for(int i = 0; i < argc; i++)
    {
        str_argv = std::string(argv[i]);
        
        if(str_argv.compare(0, 2, "-o") == 0)
        {
            redirect = true;
            redir_ip = std::string(argv[i + 1]);
            i++;
            continue;
        }
        else if(str_argv.compare(0, 2, "-r") == 0)
        {
            reload_zones = true;
        }
        else if(str_argv.compare(0, 2, "-q") == 0)
        {
            no_bind_cfg = true;
        }
        else if(str_argv.compare(0, 2, "-a") == 0)
        {
            custom_zone = true;
            zone_file = std::string(argv[i + 1]);
            i++;
            continue;
        }
        else if(str_argv.compare(0, 2, "-l") == 0)
        {
            link_zone = true;
        }
        else if(str_argv.compare(0, 2, "-h") == 0 || str_argv.compare(0, 6, "--help") == 0)
        {
            show_hints = true;
        }
    }
    
    if(show_hints)
    {
        std::cout << "Purpose:\nConvert one or more Lists of Domains into BIND Zones for Blocking or Redirection.\n\n";
        std::cout << "Behaviour:\nRead in all Domains, Remove duplicated Entries, Create a Zone Configuration with all Domains "
                "pointing to Null Zone, Create a Null Zone File resolving to Localhost (or optional IP), add include Line in "
                "BIND Configuration if not already exists. Reload the Bind Configuration to apply new Zones.\n\n";
        std::cout << "Usage:\nblocklist-generator <\"Domain List1|Domain ListN\"> <ZoneListFile> <NullZoneFile> <BindCfgFile> <Options>" << std::endl;
        std::cout << "\nOptions:\n"
                "\t-o <IP-Address> : Redirect to the given IP instead of Localhost\n"
                "\t-r : Automatically reload Zone Files in BIND after creation\n"
                "\t-q : Do not edit BIND Configuration File. Manual Include required!\n"
                "\t-a <ZoneFile> : Use the given Null Zone File instead of generating one\n"
                "\t-l : Requires -a Option! Reference the Null Zone File instead of coping it\n"
                "\t-h, --help : Show Help and more Information\n";
        std::cout << "\nDomain List File:\n- Single Entry per Line\n- Use Domain Names only\n- Use default Text Files\n"
                "- Recommended: Use seperate Files for custom Domains and dynamic Lists (from Web etc.)\n\n"
                "Example Domain List File Entries:\n101com.com\ngoogleadservice.com\nadthis.com\n\n"
                "Obtain Plain Text Advertising Domain Lists:\nhttp://pgl.yoyo.org/adservers/serverlist.php?hostformat=;showintro=0"<< std::endl;
        
        return 0;
    }
    
    if(argc < 4)
    {
        std::cout << "Not all required Parameters have been set!\n\nUsage:\nblocklist-generator"
                "<Domain List1|Domain ListN> <ZoneListFile> <ZoneFile> <BindCfgFile> <Options>\n\nUse: -h or --help for more Information and Help." << std::endl;
        
        return -1;
    }
    std::string domain_list_seperator = "|";
    std::string str_domains = std::string(argv[1]);
    std::vector<std::string> domain_files = split(str_domains, domain_list_seperator);
    std::string str_zones = std::string(argv[2]);
    std::string str_zonefile = std::string(argv[3]);
    std::string str_bindfile = std::string(argv[4]);
    
    
    // Read domain names
    std::vector<std::string> domains;
    
    for(unsigned long int i = 0; i < domain_files.size(); i++)
    {
        std::cout << "Domain List: " << domain_files.at(i) << std::endl;
        std::vector<std::string> tmp_list = read_file(domain_files.at(i));
        domains.insert(domains.end(), tmp_list.begin(), tmp_list.end());
    }
    
    std::cout << std::endl;
    
    // Remove Duplicates
    domains = remove_duplicates(domains);
    
    // Create Null Zone File
    if(custom_zone)
    {
        if(link_zone)
        {
            write_blocklist(str_zones, zone_file, domains);
        }
        else
        {
            write_blocklist(str_zones, str_zonefile, domains);
            copy_file(zone_file, str_zonefile);
        }
    }
    else
    {
        write_blocklist(str_zones, str_zonefile, domains);
        write_zonefile(str_zonefile, redir_ip);
    }
    
    // Edit BIND Config
    if(!no_bind_cfg)
    {
        std::cout << "Editing BIND Configuration " << str_bindfile << "\n";
        edit_bind_cfg(str_bindfile, str_zones);
    }
    
    // Reload BIND
    if(reload_zones)
    {
        std::cout << "Reloading BIND Zone List...\n";
        system("rndc reload");
    }
    
    std::cout << "\n\n...finished successfully!" << std::endl;
    
    return 0;
}

