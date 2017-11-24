/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
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
#include <graphene/chain/lock_balance_evaluator.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/is_authorized_asset.hpp>
#include <graphene/chain/locked_balance_object.hpp>

namespace graphene { namespace chain {
   using namespace boost::multiprecision;
   
   void_result lock_balance_evaluator::do_evaluate( const lock_balance_operation & op )
   {
      try {
      
         const database& d = db();
         const asset_object&   asset_type       = op.amount.asset_id(d);
         const asset_lock_data_object & lock_data_obj=asset_type.lock_data(d);
         
         //const asset_lock_data_object lock_data = asset_type.lock_data(d);

         FC_ASSERT(asset_type.validate_lock_option(),"asset ${symbol} no lock option ");
         
         bool insufficient_balance = d.get_balance( op.issuer, op.amount.asset_id ).amount >= op.amount.amount;
         FC_ASSERT(insufficient_balance,
                   "Insufficient Balance: unable to lock balance"
                   );
         
         profit=lock_data_obj.get_profit(op.amount.amount,op.period,d);
         to_locked_balance=profit+op.amount.amount;
         bool insufficient_pool=(asset_type.lock_data(d).interest_pool>=profit);

         FC_ASSERT(insufficient_pool,
                   "Insufficient interest pool: unable to lock balance"
                   );

   }  FC_CAPTURE_AND_RETHROW( (op) ) }
   
   void_result lock_balance_evaluator::do_apply( const lock_balance_operation& o )
   {
      try {
         database& d = db();
         const asset_object&   asset_type       = o.amount.asset_id(d);
         const asset_dynamic_data_object asset_dynamic_data_o=asset_type.dynamic_data(d);
         const asset_lock_data_object & lock_data_obj=asset_type.lock_data(d);
         
         const auto new_locked_balance_o =d.create<locked_balance_object>([&](locked_balance_object &obj){
            obj.initial_lock_balance=o.amount.amount;
            obj.locked_balance=to_locked_balance;
            obj.lock_period=o.period;
            obj.lock_type=locked_balance_object::userSet;
			obj.asset_id = o.amount.asset_id;
            obj.lock_time=d.get_dynamic_global_properties().time.sec_since_epoch();
         });
         
         
         auto& index = d.get_index_type<account_balance_index>().indices().get<by_account_asset>();
         auto itr = index.find(boost::make_tuple(o.issuer, o.amount.asset_id));

         FC_ASSERT( itr!=index.end(), "Insufficient Balance");

		 d.adjust_balance(o.issuer, -o.amount);
         
         d.modify(*itr,[&](account_balance_object & obj){
            obj.add_lock_balance(new_locked_balance_o.id);
         });
         
         d.modify(lock_data_obj,[&](asset_lock_data_object &obj){            
			 fc::uint128_t locking_coin_day = fc::uint128_t(o.amount.amount.value) * fc::uint128_t(o.period);
            obj.interest_pool-=profit;
			FC_ASSERT(fc::uint128_t::max_value() - locking_coin_day > obj.lock_coin_day, "invalid locking balance and period");
			obj.lock_coin_day += locking_coin_day;
         });

         
         d.modify(asset_dynamic_data_o, [&](asset_dynamic_data_object &obj){
            obj.locked_balance+=to_locked_balance;
         });

      return void_result();
   } FC_CAPTURE_AND_RETHROW( (o) ) }
   
   void_result set_lock_data_evaluator::do_evaluate( const set_lock_data_operation & op )
   { try {
      
      const database& d = db();
      
      const asset_object&   asset_type =op.init_interest_pool.asset_id(d);

	  //if the pool is exist,will set failure
	  auto& index = d.get_index_type<asset_index>().indices().get<by_id>();
	  const auto& itrs = index.find(op.init_interest_pool.asset_id);
	  
	  FC_ASSERT( itrs != index.end(), "asset id not exist");

	  const asset_object& asset_obj = *itrs;

	  FC_ASSERT(!asset_obj.lock_data_id.valid(), "lock data already created!");

	//  if ()
      
      FC_ASSERT(asset_type.issuer==op.issuer,
                "operation issuer ${op.issuer} is not asset issuer ${asset_type.issuer}"
      );
      
      if(op.init_interest_pool.amount>0){
         bool insufficient_balance = d.get_balance( op.issuer, op.init_interest_pool.asset_id ).amount >= op.init_interest_pool.amount;
         FC_ASSERT(insufficient_balance,
                   "Insufficient Balance: unable to pure to init lock interest pool");
      }
      return void_result();
         
      
   }  FC_CAPTURE_AND_RETHROW( (op) ) }
   
   void_result set_lock_data_evaluator::do_apply( const set_lock_data_operation& o )
   { try {
	   database& d = db();
      
	  FC_ASSERT(d.get_balance(o.issuer, o.init_interest_pool.asset_id) >= o.init_interest_pool,
		  "No enough balance pay for pool");
	       
	  if (o.init_interest_pool.amount>0)
		  d.adjust_balance(o.issuer, -o.init_interest_pool);

      const auto new_lock_data_o = d.create<asset_lock_data_object>([&](asset_lock_data_object& obj){
         obj.interest_pool=o.init_interest_pool.amount;
         auto asset_id=o.init_interest_pool.asset_id;
         obj.nominal_interest_perday=Interest(asset(FCC_INTEREST_BASE_SUPPLY,asset_id),asset(o.nominal_interest_perday,asset_id));
         obj.reward_coefficient=o.reward_coefficient;
         obj.lock_coin_day=0;
      });
      
      const asset_object&   asset_type =o.init_interest_pool.asset_id(d);
      
      d.modify(asset_type,[&](asset_object& obj){
         obj.lock_data_id=new_lock_data_o.id;

      });
      
      return void_result();
   } FC_CAPTURE_AND_RETHROW( (o) ) }


   void_result unlock_balance_evaluator::do_evaluate(const unlock_balance_operation & o)
   {
	   try {
		   const database& d = db();
		   for (auto i = 0; i < o.lockeds.size();i++)
		   {
			   const locked_balance_object & item = o.lockeds[i].locked_id(d);
			   FC_ASSERT(item.locked_balance  >= 0,
				   "Insufficient interest pool: unable to unlock balance"
				   );
		   }
	   }  FC_CAPTURE_AND_RETHROW((o))
   }

   void_result unlock_balance_evaluator::do_apply(const unlock_balance_operation& o)
   {
	   try {
		   database& d = db();
		   auto& index = d.get_index_type<account_balance_index>().indices().get<by_account_asset>();
		   
		   for (auto  itr = o.lockeds.begin(); itr != o.lockeds.end(); itr++)
		   {
			   const locked_balance_object & item = itr->locked_id(d);
			   const asset_lock_data_object & lock_data_obj = item.asset_id(d).lock_data(d);

			   auto find = index.find(boost::make_tuple(o.issuer, item.asset_id));

			   FC_ASSERT(find != index.end(), "Insufficient Balance");
			   FC_ASSERT(!item.finish, "already unlocked balance"); // check further		   
			   

			   if (itr->expired && (d.head_block_time() >= time_point_sec(item.get_unlock_time())))
			   {
				   d.adjust_balance(o.issuer, asset(item.locked_balance,item.asset_id));
				   d.modify(lock_data_obj, [&](asset_lock_data_object &obj){
					   obj.lock_coin_day -= fc::uint128_t(item.initial_lock_balance.value) * fc::uint128_t(item.lock_period.value);
				   });
			   }
			   else
			   {
				   d.adjust_balance(o.issuer, asset(item.initial_lock_balance, item.asset_id));
				   d.modify(lock_data_obj, [&](asset_lock_data_object &obj){
					   obj.interest_pool += item.locked_balance - item.initial_lock_balance;
					   obj.lock_coin_day -= fc::uint128_t(item.initial_lock_balance.value) * fc::uint128_t(item.lock_period.value);
				   });
			   }


			   //update the lock_balance obj status to db.
			   d.modify(item, [&](locked_balance_object & obj){
				   obj.finish = true;
			   });
			   
			   d.modify(*find, [&](account_balance_object & obj){
				   obj.unlock_balance(item.id);
			   });
		   }

	   } FC_CAPTURE_AND_RETHROW((o))
   }

   void_result donation_balance_evaluator::do_evaluate(const donation_balance_operation & o)
   {
	   try {
		   const database& d = db();
		   FC_ASSERT(d.get_balance(o.issuer, o.amount.asset_id) >= o.amount,"no enough balance");
	
	   }  FC_CAPTURE_AND_RETHROW((o))
   }

   void_result donation_balance_evaluator::do_apply(const donation_balance_operation& o)
   {
	   try {
		   database& d = db();
		   auto& index = d.get_index_type<asset_index>().indices().get<by_id>();
		   const auto& itrs = index.find(o.amount.asset_id);
		   FC_ASSERT(itrs != index.end(), "asset id not exist");
		   const asset_lock_data_object & lock_data_obj = itrs->lock_data(d);

		   d.adjust_balance(o.issuer, -o.amount);
		   d.modify(lock_data_obj, [&](asset_lock_data_object &obj){
			   obj.interest_pool += o.amount.amount;
		   });

	   } FC_CAPTURE_AND_RETHROW((o))
   }

} } // graphene::chain
