/*
 * Copyright (c) 2017 FinChain, Inc., and contributors.
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
#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/memo.hpp>

namespace graphene { namespace chain {

   /**
    * @ingroup operations
    *
    * @brief lock an amount of one asset for period to get profile gived by system or ...
    *
    *  Fees are paid by the "from" account
    *
    *  @pre amount.amount > 0
    *  @pre fee.amount >= 0
    *  @pre from != to
    *  @post from account's balance will be reduced by fee and amount
    *  @post to account's balance will be increased by amount
    *  @return n/a
    */
   struct lock_balance_operation : public base_operation
   {
      struct fee_parameters_type {
         uint64_t fee       = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
         uint32_t price_per_kbyte = 10 * GRAPHENE_BLOCKCHAIN_PRECISION; /// only required for large memos.
      };

      asset            fee;
      /// Account that lock balance
      account_id_type  issuer;

      /// The amount of asset to operation
      asset            amount;

      uint32_t         period;
      extensions_type   extensions;

      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k)const;
   };
   
   struct set_lock_data_operation : public base_operation
   {
      struct fee_parameters_type {
         uint64_t fee       = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
         uint32_t price_per_kbyte = 10 * GRAPHENE_BLOCKCHAIN_PRECISION; /// only required for large memos.
      };
      
      asset             fee;
      /// Account that lock balance
      account_id_type   issuer;
      uint64_t          nominal_interest_perday; //
      uint16_t          reward_coefficient;
      asset             init_interest_pool;
	  uint32_t			max_period;
      extensions_type   extensions;
      
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k)const;
   };
   
   struct unlock_balance_operation : public base_operation
   {
      struct fee_parameters_type {
         uint64_t fee       = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
         uint32_t price_per_kbyte = 10 * GRAPHENE_BLOCKCHAIN_PRECISION; /// only required for large memos.
      };
            
      struct unlock_detail{
         locked_balance_id_type  locked_id;
		 bool             expired;
      };
      
      asset            fee;
      /// Account that lock balance
      account_id_type  issuer;
      
      /// The amount of asset to operation
      vector<unlock_detail>            lockeds;
      
      extensions_type   extensions;
      
      account_id_type fee_payer()const { return issuer; }
      void            validate()const;
      share_type      calculate_fee(const fee_parameters_type& k)const;
   };

   struct donation_balance_operation : public base_operation
   {
	   struct fee_parameters_type {
		   uint64_t fee = 20 * GRAPHENE_BLOCKCHAIN_PRECISION;
		   uint32_t price_per_kbyte = 10 * GRAPHENE_BLOCKCHAIN_PRECISION; /// only required for large memos.
	   };

	   asset            fee;
	   /// Account that lock balance
	   account_id_type  issuer;

	   /// The amount of asset to operation
	   asset            amount;

	   extensions_type   extensions;

	   account_id_type fee_payer()const { return issuer; }
	   void            validate()const;
	   share_type      calculate_fee(const fee_parameters_type& k)const;
   };
   
}} // graphene::chain


FC_REFLECT( graphene::chain::lock_balance_operation::fee_parameters_type, (fee)(price_per_kbyte) )
FC_REFLECT(graphene::chain::lock_balance_operation, (fee)(issuer)(amount)(period)(extensions))

FC_REFLECT( graphene::chain::set_lock_data_operation::fee_parameters_type, (fee)(price_per_kbyte) )
FC_REFLECT(graphene::chain::set_lock_data_operation, (fee)(issuer)(nominal_interest_perday)(reward_coefficient)(init_interest_pool)(max_period)(extensions))


FC_REFLECT( graphene::chain::unlock_balance_operation::fee_parameters_type, (fee)(price_per_kbyte) )
FC_REFLECT(graphene::chain::unlock_balance_operation::unlock_detail, (locked_id)(expired))
FC_REFLECT( graphene::chain::unlock_balance_operation, (fee)(issuer)(lockeds)(extensions) )

FC_REFLECT(graphene::chain::donation_balance_operation::fee_parameters_type, (fee)(price_per_kbyte))
FC_REFLECT(graphene::chain::donation_balance_operation, (fee)(issuer)(amount)(extensions))
