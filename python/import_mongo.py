import pymongo
from pymongo import MongoClient
from bson.json_util import loads
from bson.json_util import dumps
import pprint

client = MongoClient()
client = MongoClient('localhost', 27017)
print (client)

dbname = 'McStas'
collectionname = 'requests'
jsonfilename='request_new.json'

db = client[dbname]
collection = db[collectionname]
print(collection)
print()

print("Reading request from json file: {}".format(jsonfilename))
bsonObj = loads(open(jsonfilename).read())
print(bsonObj)
collection.insert_one(bsonObj)


j = { "source" : {"lambda":  4.51}}
instrument =  {"instrument" : { "name": "D22"}}

print("\n\nFind entries with:")

print("------------------------------")
print(j)
print(instrument)
print(
    collection.find_one({ "$and":
                         [ instrument,
                           j]
    }
    )
)




"""
dbname = 'openPMD'
collectionname = 'zsolt'
db = client[dbname]
collection = db[collectionname]
print(collection)
jsonfilename='NeutronData_simple.json'
json_file= open(jsonfilename)
bsonObj = loads(json_file.read())

#print(bsonObj)

#collection.insert_one(bsonObj)
print(collection)



"""
