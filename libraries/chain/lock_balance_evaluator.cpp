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

namespace graphene { namespace chain {
   void_result lock_balance_evaluator::do_evaluate( const lock_balance_operation & op )
   {
      try {
      
         const database& d = db();
         const account_object& issuer           = op.issuer(d);
         const asset_object&   asset_type       = op.amount.asset_id(d);
         //const asset_lock_data_object lock_data = asset_type.lock_data(d);

         FC_ASSERT(asset_type.validate_lock_option(),"asset ${symbol} no lock option ");
         
         bool insufficient_balance = d.get_balance( op.issuer, op.amount.asset_id ).amount >= op.amount.amount;
         FC_ASSERT(insufficient_balance,
                   "Insufficient Balance: unable to lock balance"
                   );

   }  FC_CAPTURE_AND_RETHROW( (op) ) }
   
   void_result lock_balance_evaluator::do_apply( const lock_balance_operation& o )
   {
      try {
         database& d = db();
         const asset_object&   asset_type       = o.amount.asset_id(d);
         const asset_lock_data_object lock_data_obj=asset_type.lock_data(d);
         const auto instrest=lock_data_obj.get_interest(o.period);
         const share_type locked_balance=instrest*o.amount.amount;
         
         const auto new_locked_balance_o =d.create<locked_balance_object>([&](locked_balance_object &obj){
         });
         
         //const auto new_locked_balance_o = d.create<locked_balance_object>([&](locked_balance_object& obj){
         //   obj.locked_balance=locked_balance;
            
         //});
         
      return void_result();
   } FC_CAPTURE_AND_RETHROW( (o) ) }
   
   void_result set_lock_data_evaluator::do_evaluate( const set_lock_data_operation & op )
   { try {
      
      const database& d = db();
      
      const asset_object&   asset_type =op.init_interest_pool.asset_id(d);

      
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
      if(o.init_interest_pool.amount>0)
         db().adjust_balance( o.issuer, -o.init_interest_pool);
      
      
      database& d = db();
      const auto new_lock_data_o = d.create<asset_lock_data_object>([&](asset_lock_data_object& obj){
         obj.interest_pool=o.init_interest_pool.amount;
         obj.nominal_interest_rate=o.nominal_interest_rate;
         obj.reward_coefficient=o.reward_coefficient;
         obj.lock_coin_day=0;
      });
      
      const asset_object&   asset_type =o.init_interest_pool.asset_id(d);
      
      d.modify(asset_type,[&](asset_object& obj){
         obj.lock_data_id=new_lock_data_o.id;

      });
      
      return void_result();
   } FC_CAPTURE_AND_RETHROW( (o) ) }

} } // graphene::chain
