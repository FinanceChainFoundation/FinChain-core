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
#include <graphene/chain/protocol/lock_balance.hpp>

namespace graphene { namespace chain {

share_type lock_balance_operation::calculate_fee( const fee_parameters_type& schedule )const
{
   share_type core_fee_required = schedule.fee;

   return core_fee_required;
}


void lock_balance_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT( amount.amount > 0 );
   FC_ASSERT( (period >= FCC_INTEREST_DAY) && (period <= 2*FCC_INTEREST_YEAR),"lock period should longer than one day and short than 2 years");
}
   
share_type set_lock_data_operation::calculate_fee( const fee_parameters_type& schedule )const
{
   share_type core_fee_required = schedule.fee;
   
   return core_fee_required;
}


void set_lock_data_operation::validate()const
{
   FC_ASSERT( fee.amount >= 0 );
   FC_ASSERT(nominal_interest_perday>0);
   FC_ASSERT(init_interest_pool.amount>=0);
   FC_ASSERT(reward_coefficient <= GRAPHENE_100_PERCENT);   
   FC_ASSERT(max_period >= 2); // minimum 2 days 
}

share_type unlock_balance_operation::calculate_fee(const fee_parameters_type& schedule)const
{
	share_type core_fee_required = schedule.fee;

	return core_fee_required;
}


void unlock_balance_operation::validate()const
{
	FC_ASSERT(fee.amount >= 0);
}

share_type donation_balance_operation::calculate_fee(const fee_parameters_type& schedule)const
{
	share_type core_fee_required = schedule.fee;

	return core_fee_required;
}


void donation_balance_operation::validate()const
{
	FC_ASSERT(fee.amount >= 0);
}

} } // graphene::chain
