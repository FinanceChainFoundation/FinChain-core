/*
 * Copyright (c) 2015 FinChain, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <boost/test/unit_test.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/market_object.hpp>
#include <graphene/chain/vesting_balance_object.hpp>
#include <graphene/chain/withdraw_permission_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/locked_balance_object.hpp>

#include <fc/crypto/digest.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( operation_tests, database_fixture )

/*
BOOST_AUTO_TEST_CASE(lock_balance_test)
{
	try {
		ACTORS((dan)(nathan));
		const auto& core = asset_id_type()(db);
		const auto & core_lock = core.lock_data(db);
		auto total = get_balance(committee_account, asset_id_type());
		uint64_t UNIT = GRAPHENE_MAX_SHARE_SUPPLY / 10;
		uint64_t dan_balance;
		BOOST_CHECK_EQUAL(total, GRAPHENE_MAX_SHARE_SUPPLY);
		BOOST_CHECK_EQUAL(get_balance(dan_id, asset_id_type()), 0);
		transfer(committee_account, dan_id, asset(UNIT * 7));
		transfer(committee_account, nathan_id, asset(UNIT * 2));
		transfer(committee_account, account_id_type(3), asset(UNIT));

		auto lock_balance = [&](asset amount, uint32_t period)
		{
			lock_balance_operation op;
			op.fee = asset();
			op.issuer = dan_id;
			op.amount = amount;
			op.period = period;
			trx.operations.push_back(op);
			set_expiration(db, trx);
			sign(trx, dan_private_key);
			db.push_transaction(trx);
			trx.clear();
			generate_block();
			verify_asset_supplies(db);
		};

		auto unlock_balance = [&](int64_t current_balance, bool expired, bool has_interest)
		{
			int64_t result = 0;
			unlock_balance_operation op;
			op.fee = asset();
			op.issuer = dan_id;
			const auto & ids = db.get_locked_balance_ids(dan_id, asset_id_type());
			FC_ASSERT(ids.size() > 0, "dan didn't has locked balance!");

			unlock_balance_operation::unlock_detail one;
			one.locked_id = ids.at(0);
			one.expired = expired;
			op.locked = one;
			trx.operations.push_back(op);
			set_expiration(db, trx);
			sign(trx, dan_private_key);
			db.push_transaction(trx);
			trx.clear();

			const locked_balance_object & b_obj = one.locked_id(db);
			generate_block();
			if (has_interest)
			{
				BOOST_CHECK_EQUAL(get_balance(dan_id, asset_id_type()), int64_t(current_balance + b_obj.locked_balance.value));
				result = b_obj.locked_balance.value - b_obj.initial_lock_balance.value;
			}
			else
			{
				BOOST_CHECK_EQUAL(get_balance(dan_id, asset_id_type()), int64_t(current_balance + b_obj.initial_lock_balance.value));
			}
			verify_asset_supplies(db);
			return result;
		};
		//create_lock_able_asset(committee_account,asset_id_type(0),50,45,300000000LL*100000LL);
		{
			donation_balance_operation op;
			op.fee = asset();
			op.issuer = dan_id;
			op.amount = asset(UNIT * 5);

			trx.operations.push_back(op);

			sign(trx, dan_private_key);
			db.push_transaction(trx);
			trx.clear();
			generate_block();
		}
		dan_balance = UNIT * 2;
		BOOST_CHECK_EQUAL(get_balance(dan_id, asset_id_type()), dan_balance);
		BOOST_CHECK_EQUAL(core_lock.interest_pool.value, UNIT * 5);
#if 1
		verify_asset_supplies(db);

		lock_balance(asset(UNIT), JRC_INTEREST_DAY);
		dan_balance -= UNIT;

		BOOST_CHECK_EQUAL(get_balance(dan_id, asset_id_type()), dan_balance);
		verify_asset_supplies(db);
		generate_block(~0, generate_private_key("null_key"), 28800);
		dan_balance += unlock_balance(UNIT, true, true);

		lock_balance(asset(UNIT), JRC_INTEREST_DAY);
		unlock_balance(dan_balance, false, false);


		lock_balance(asset(UNIT), JRC_INTEREST_DAY);
		generate_block(~0, generate_private_key("null_key"), 28801);
		unlock_balance(dan_balance, false, false);

		lock_balance(asset(UNIT), 720 * JRC_INTEREST_DAY);
		generate_block(~0, generate_private_key("null_key"), 1000);
		unlock_balance(dan_balance, true, false);

		lock_balance(asset(UNIT), 720 * JRC_INTEREST_DAY);
		generate_block(~0, generate_private_key("null_key"), 28800 * 720);
		unlock_balance(dan_balance, true, true);
		BOOST_CHECK_EQUAL(get_balance(dan_id, asset_id_type()), dan_balance);
#endif

	}
	catch (fc::exception& e) {
		edump((e.to_detail_string()));
		throw;
	}
}


//test for user issue asset's lock balance function
BOOST_AUTO_TEST_CASE(lock_balance_test2)
{
	try {
		ACTORS((dan));
		uint64_t UNIT = GRAPHENE_MAX_SHARE_SUPPLY / 10;
		uint64_t dan_balance;
		
		transfer(committee_account, dan_id, asset(UNIT * 7));
		//transfer(committee_account, nathan_id, asset(UNIT * 2));
		//transfer(committee_account, account_id_type(3), asset(UNIT));
		auto has_asset = [&](std::string symbol) -> bool
		{
			const auto& assets_by_symbol = db.get_index_type<asset_index>().indices().get<by_symbol>();
			return assets_by_symbol.find(symbol) != assets_by_symbol.end();
		};

		BOOST_CHECK(!has_asset("TGHTT"));

		create_user_issued_asset("TGHTT", dan_id(db), 0);

		BOOST_CHECK(has_asset("TGHTT"));
		const auto& ast = *db.get_index_type<asset_index>().indices().get<by_symbol>().find("TGHTT");
		const auto & ast_id = ast.get_id();
		trx.clear();
		issue_uia(dan, ast.amount(UNIT * 10));
		
		generate_block();
		trx.clear();
		create_lock_able_asset(dan_id, ast_id, 1001126926400LL, 2000, 0LL, 720);
		generate_block();
		
		
		auto lock_balance = [&](asset amount, uint32_t period)
		{
			lock_balance_operation op;
			op.fee = asset();
			op.issuer = dan_id;
			op.amount = amount;
			op.period = period;
			trx.operations.push_back(op);
			set_expiration(db, trx);
			sign(trx, dan_private_key);
			db.push_transaction(trx);
			trx.clear();
			generate_block();
			verify_asset_supplies(db);
		};

		auto unlock_balance = [&](int64_t current_balance,bool expired,bool has_interest)
		{
			int64_t result = 0;
			unlock_balance_operation op;
			op.fee = asset();
			op.issuer = dan_id;
			const auto & ids = db.get_locked_balance_ids(dan_id, ast_id);
			FC_ASSERT(ids.size() > 0, "dan didn't has locked balance!");

			unlock_balance_operation::unlock_detail one;
			one.locked_id = ids.at(0);
			one.expired = expired;
			op.locked = one;
			trx.operations.push_back(op);
			set_expiration(db, trx);
			sign(trx, dan_private_key);
			db.push_transaction(trx);
			trx.clear();

			const locked_balance_object & b_obj = one.locked_id(db);
			generate_block();
			if (has_interest)
			{ 
				BOOST_CHECK_EQUAL(get_balance(dan_id, ast_id), int64_t(current_balance + b_obj.locked_balance.value));
				result = b_obj.locked_balance.value - b_obj.initial_lock_balance.value;
			}
			else
			{
				BOOST_CHECK_EQUAL(get_balance(dan_id, ast_id), int64_t(current_balance + b_obj.initial_lock_balance.value));
			}
			verify_asset_supplies(db);
			return result;
		};
		//create_lock_able_asset(committee_account,asset_id_type(0),50,45,300000000LL*100000LL);
		{
			donation_balance_operation op;
			op.fee = asset();
			op.issuer = dan_id;
			op.amount = asset(UNIT/2, ast_id);

			trx.operations.push_back(op);

			sign(trx, dan_private_key);
			db.push_transaction(trx);
			trx.clear();
			generate_block();
		}
		//dan_balance = UNIT * 2;
		//BOOST_CHECK_EQUAL(get_balance(dan_id, ast_id), dan_balance);
		
		//const asset_lock_data_object & obj = (*(ast_id(db).lock_data_id))(db);
		//BOOST_CHECK_EQUAL(obj.interest_pool.value, UNIT * 5);
		
		verify_asset_supplies(db);

		lock_balance(asset(UNIT/10, ast_id), 720*JRC_INTEREST_DAY);
		//dan_balance -= UNIT;
		
		BOOST_CHECK_EQUAL(get_balance(dan_id, ast_id), dan_balance);
		verify_asset_supplies(db);
		generate_block(~0, generate_private_key("null_key"), 28800);
		dan_balance += unlock_balance(UNIT, true, true);

		lock_balance(asset(UNIT, ast_id), JRC_INTEREST_DAY);
		unlock_balance(dan_balance, false, false);
		

		lock_balance(asset(UNIT, ast_id), JRC_INTEREST_DAY);
		generate_block(~0, generate_private_key("null_key"), 28801);
		unlock_balance(dan_balance, false, false);

		lock_balance(asset(UNIT, ast_id), 720 * JRC_INTEREST_DAY);
		generate_block(~0, generate_private_key("null_key"), 1000);
		unlock_balance(dan_balance, true, false);
		
		lock_balance(asset(UNIT, ast_id), 100 * JRC_INTEREST_DAY);
		generate_block(~0,generate_private_key("null_key"),28800 * 100);
		unlock_balance(dan_balance, true, true);
		BOOST_CHECK_EQUAL(get_balance(dan_id, ast_id), dan_balance);
	}
	catch (fc::exception& e) {
		edump((e.to_detail_string()));
		throw;
	}
}
*/
//test for asset presale
BOOST_AUTO_TEST_CASE(asset_presale_test)
{
	try {
		ACTORS((core)(first)(second)(third));
		uint64_t UNIT = GRAPHENE_MAX_SHARE_SUPPLY / 10;
		uint64_t dan_balance;

		transfer(committee_account, core_id, asset(UNIT * 2));
		transfer(committee_account, first_id, asset(UNIT * 2));
		transfer(committee_account, second_id, asset(UNIT * 2));
		transfer(committee_account, third_id, asset(UNIT * 2));
		auto has_asset = [&](std::string symbol) -> bool
		{
			const auto& assets_by_symbol = db.get_index_type<asset_index>().indices().get<by_symbol>();
			return assets_by_symbol.find(symbol) != assets_by_symbol.end();
		};

		BOOST_CHECK(!has_asset("FIRST"));
		create_user_issued_asset("FIRST", first_id(db), 0);
		BOOST_CHECK(has_asset("FIRST"));
		auto a1 = *db.get_index_type<asset_index>().indices().get<by_symbol>().find("FIRST");
		auto  a1_id = a1.get_id();
		trx.clear();
		issue_uia(first, a1.amount(GRAPHENE_MAX_SHARE_SUPPLY));

		generate_block();
		trx.clear();


		BOOST_CHECK(!has_asset("SECOND"));
		create_user_issued_asset("SECOND", second_id(db), 0);
		BOOST_CHECK(has_asset("SECOND"));
		auto a2 = *db.get_index_type<asset_index>().indices().get<by_symbol>().find("SECOND");
		auto  a2_id = a2.get_id();
		trx.clear();
		issue_uia(get_account("second"), a2.amount(GRAPHENE_MAX_SHARE_SUPPLY));

		generate_block();
		trx.clear();


		BOOST_CHECK(!has_asset("THIRD"));
		create_user_issued_asset("THIRD", third_id(db), 0);
		BOOST_CHECK(has_asset("THIRD"));
		auto a3 = *db.get_index_type<asset_index>().indices().get<by_symbol>().find("THIRD");
		auto a3_id = a3.get_id();
		trx.clear();
		issue_uia(get_account("third"), a3.amount(GRAPHENE_MAX_SHARE_SUPPLY));

		generate_block();
		trx.clear();
		
		generate_block();


		auto create_presale = [&]()
		{
			asset_presale_create_operation op;
			op.fee = asset();
			op.issuer = first_id;
			op.start = db.head_block_time() + 15;
			op.stop = db.head_block_time() + 86400;
			op.asset_id = a1_id;
			op.amount = GRAPHENE_MAX_SHARE_SUPPLY/10;
			op.early_bird_part = op.amount / GRAPHENE_100_PERCENT  * GRAPHENE_1_PERCENT * 50;
			op.asset_of_top = a2_id;
			op.soft_top = GRAPHENE_MAX_SHARE_SUPPLY / 50;
			op.hard_top = GRAPHENE_MAX_SHARE_SUPPLY / 2;
			op.lock_period = 86400;
			op.unlock_type = 1;
			op.mode = 0;

			op.early_bird_pecents[db.head_block_time() + 1 * 86400] = GRAPHENE_1_PERCENT * 150;
			op.early_bird_pecents[db.head_block_time() + 7 * 86400] = GRAPHENE_1_PERCENT * 140;
			op.early_bird_pecents[db.head_block_time() + 15 * 86400] = GRAPHENE_1_PERCENT * 130;
			op.early_bird_pecents[db.head_block_time() + 30 * 86400] = GRAPHENE_1_PERCENT * 120;

			op.accepts.push_back({ a2_id, GRAPHENE_MAX_SHARE_SUPPLY / 20, 1100000000000LL, GRAPHENE_MAX_SHARE_SUPPLY / 100000, GRAPHENE_MAX_SHARE_SUPPLY / 2 });
			op.accepts.push_back({ a3_id, GRAPHENE_MAX_SHARE_SUPPLY / 20, 1200000000000LL, GRAPHENE_MAX_SHARE_SUPPLY / 100000, GRAPHENE_MAX_SHARE_SUPPLY / 2 });

			trx.operations.push_back(op);
			set_expiration(db, trx);
			sign(trx, first_private_key);
			db.push_transaction(trx);
			trx.clear();
			generate_block();
			//verify_asset_supplies(db);
		};
		
		auto buy_presale= [&]()
		{
			a1 = *db.get_index_type<asset_index>().indices().get<by_symbol>().find("FIRST");
			asset_presale_buy_operation op;
			op.fee = asset();
			op.issuer = second_id;
			op.amount = asset(GRAPHENE_MAX_SHARE_SUPPLY / 50,a2_id);
			op.presale = a1.presales[0];
			trx.operations.push_back(op);
			set_expiration(db, trx);
			sign(trx, second_private_key);
			db.push_transaction(trx);
			trx.clear();
			generate_block();
			//verify_asset_supplies(db);
		};

		auto claim_presale = [&]()
		{
			a1 = *db.get_index_type<asset_index>().indices().get<by_symbol>().find("FIRST");
			asset_presale_claim_operation op;
			op.fee = asset();
			op.issuer = second_id;
			op.presale = a1.presales[0];
			trx.operations.push_back(op);
			set_expiration(db, trx);
			sign(trx, second_private_key);
			db.push_transaction(trx);
			trx.clear();
			generate_block();
			//verify_asset_supplies(db);
		};
		
		create_presale();

		auto b1 = get_balance(first_id, a1_id);
		auto b2 = get_balance(first_id, a2_id);
		auto b3 = get_balance(first_id, a3_id);

		generate_block(~0, generate_private_key("null_key"), 20);
		buy_presale();

		auto c1 = get_balance(second_id, a1_id);
		auto c2 = get_balance(second_id, a2_id);
		auto c3 = get_balance(second_id, a3_id);
		generate_block(~0, generate_private_key("null_key"), 28800*2);

		claim_presale();

		auto d1 = get_balance(first_id, a1_id);
		auto d2 = get_balance(first_id, a2_id);
		auto d3 = get_balance(first_id, a3_id);

		auto e1 = get_balance(second_id, a1_id);
		auto e2 = get_balance(second_id, a2_id);
		auto e3 = get_balance(second_id, a3_id);
		generate_block(~0, generate_private_key("null_key"), 28800*3);

		claim_presale();

		BOOST_CHECK_EQUAL(get_balance(first_id, a1_id), GRAPHENE_MAX_SHARE_SUPPLY);
	}
	catch (fc::exception& e) {
		edump((e.to_detail_string()));
		throw;
	}
}

BOOST_AUTO_TEST_SUITE_END()
