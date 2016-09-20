# blocklist-generator
Create BIND9 Zone Files for DNS Blacklisting / Adblocker Purposes. 
Uses one or more plain Text Files with Domain Names to generate BIND9 Zone Files resolving to Localhost or any other IP Adress, 
adds them to Bind Configuration and reloads Bind to apply the new Zones.

# Purpose / Description
This little Tool is written in Order to make it as easy as possible to maintain a DNS Adblocker. Just keep a List of
Advertisers Domain Names and let the blocklist-generator do all of the work. Of course it is possible to use this Tool for
creating custom DNS Resolvs as well.

# Usage
Default:
blocklist-generator <Domain List1|Domain ListN> <ZoneListFile> <ZoneFile> <BindCfgFile> <Options>

Example:
blocklist-generator "/root/ads-auto.list|/root/ads-custom.list" "/etc/bind/ads.list" "/etc/bind/ads.null" "/etc/bind/named.conf.local" -r

This will read in the "ads-auto.list" and "ads-custom.list" Files, generates a Zone Configuration "ads.null" and a List of Zones pointing
to that File "ads.list". The List will be included in the BIND Configuration "named.conf.local" and a automatic BIND reload will be 
performed (-r). As shown in this Example it is possible to use more than one Input File for the Tool. But remember to enclose your List 
in "" and use the "|" as Seperator!

# Options
    -o <IP-Address> : Redirect to the given IP instead of Localhost
    -r : Automatically reload Zone Files in BIND after creation
    -q : Do not edit BIND Configuration File. Manual Include required!
    -a <ZoneFile> : Use the given Null Zone File instead of generating one
    -l : Requires -a Option! Reference the Null Zone File instead of coping it
    -h, --help : Show Help and more Information\n
    
# Domain Name Files
This Tool expects plain Text Files with a single Domain Name per Line as Input. For Example:
adcolony.com
googleadservices.com
101ad.com

# Recommendations
  Use seperate Files for custom Domains and dynamic Lists (from Web etc.)
  Obtain Plain Text Advertising Domain Lists: http://pgl.yoyo.org/adservers/serverlist.php?hostformat=;showintro=0
