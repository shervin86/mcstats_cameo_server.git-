#ifndef MONGO_CACHE_HH
#define MONGO_CACHE_HH
#include "sim_request_server.hh"

#include <bsoncxx/json.hpp>
#include <cstdint>
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <vector>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/helpers.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/oid.hpp>
namespace panosc
{

/** \class mongo_cache
 \brief store and checks existence of request in mongoDB
*/
class mongo_cache
{
	public:
	mongo_cache(void)
	    : _instance(), _uri("mongodb://localhost:27017"), _client(_uri),
	      _bson(bsoncxx::builder::basic::document{})
	{
		//		_client = mongocxx::client(_uri);
		_db   = _client["McStas"];
		_coll = _db["requests"];
	};

	const mongocxx::collection &collection(void) const { return _coll; };

	auto bson(void) const { return _bson.view(); };

	void set_request(panosc::sim_request_server &request)
	{

		// add the ID from the hash
		auto build_doc = bsoncxx::builder::stream::document{};
		build_doc << "_id" << request.hash();
		build_doc << bsoncxx::builder::concatenate(bsoncxx::from_json(request.to_string()));
		_bson = build_doc << bsoncxx::builder::stream::finalize;
	}

	bool is_done(void) /// \todot why it can't be const?
	{
		const auto f = _coll.find_one(bson());
		return f.has_value();
	};

	void save_request(void)
	{
		_coll.insert_one(bson()); ///\todo check return value
	}

	private:
	mongocxx::instance _instance; // This should be done only once.
	mongocxx::uri      _uri;
	mongocxx::client   _client;

	mongocxx::database   _db;
	mongocxx::collection _coll;

	bsoncxx::document::value _bson;
	//	bsoncxx::builder::basic::document _bson;
};
} // namespace panosc
#endif
