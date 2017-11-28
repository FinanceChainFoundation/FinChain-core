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

		auto unlock_balance = [&](int64_t current_balance,bool expired,bool has_interest)
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

		verify_asset_supplies(db);

		lock_balance(asset(UNIT), FCC_INTEREST_DAY);
		dan_balance -= UNIT;
		
		BOOST_CHECK_EQUAL(get_balance(dan_id, asset_id_type()), dan_balance);
		verify_asset_supplies(db);
		generate_block(~0, generate_private_key("null_key"), 28800);
		dan_balance += unlock_balance(UNIT, true, true);

		lock_balance(asset(UNIT), FCC_INTEREST_DAY);
		unlock_balance(dan_balance, false, false);
		

		lock_balance(asset(UNIT), FCC_INTEREST_DAY);
		generate_block(~0, generate_private_key("null_key"), 28801);
		unlock_balance(dan_balance, false, false);

		lock_balance(asset(UNIT), 720 * FCC_INTEREST_DAY);
		generate_block(~0, generate_private_key("null_key"), 1000);
		unlock_balance(dan_balance, true, false);
		
		lock_balance(asset(UNIT), 720 * FCC_INTEREST_DAY);		
		generate_block(~0,generate_private_key("null_key"),28800 * 720);
		unlock_balance(dan_balance, true, true);
		BOOST_CHECK_EQUAL(get_balance(dan_id, asset_id_type()), dan_balance);
		
	}
	catch (fc::exception& e) {
		edump((e.to_detail_string()));
		throw;
	}
}

BOOST_AUTO_TEST_SUITE_END()
