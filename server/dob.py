#!/usr/bin/env python
# Access Gateway Controller
# @author: Luke Hovington

import psycopg2

# TODO get device salt and card auth list..
device_deploy_salt = "djhldfsljhfdhjlkfsd"
allowed_cards_list = [12344010]
dob_debug = False

pg_defaults = { 'host': '127.0.0.1',
                    'port': 5432,
                    'user': 'website',
                    'pass': 'th1s1sapa33w0rd',
                    'table': 'membership'}
                    
#pg_db = psycopg2.connect(database=pg_defaults['table'],
#                         user    =pg_defaults['user'],
#                         password=pg_defaults['pass'],
#                         host    =pg_defaults['host'],
#                         port    =pg_defaults['port'])

def dob_debugPrint(*x):
    if dob_debug:
        print x
                   
def dob_get_salt(deviceName):
    return device_deploy_salt

# Test to see if the device is in the database and update the info
def dob_hello_check(deviceName, macAdd, versionNumber):
    dob_debugPrint( "Testing dob Hello...." )
    return True


def dob_heartBeat_register(deviceName):
    dob_debugPrint( "Register heartbeat in dob" )
    return True
    
def dob_check_user(deviceName, cardId):
    dob_debugPrint( "checking user.." )
    return cardId in allowed_cards_list
    
#pg_activity = """
#SELECT *
#FROM 
#  "Asset_Type";
#
#"""

#pg_cursor = pg_db.cursor()
#
#pg_cursor.execute(pg_activity)
#pg_activity_data = pg_cursor.fetchall() #fetchone()
#pprint.pprint(pg_activity_data)
#
#
#pg_cursor.close()
#pg_db.close()