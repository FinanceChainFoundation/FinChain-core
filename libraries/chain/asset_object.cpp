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
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/database.hpp>

using namespace graphene::chain;
using namespace boost::multiprecision;

share_type asset_lock_data_object::get_profit(share_type tolocking_balance,uint32_t lock_period,const database &_db)const
{
	return (asset(tolocking_balance, asset_id)*_get_interest(lock_period, _db) - asset(tolocking_balance, asset_id)).amount;
}

Interest fast_pow_of_interest(Interest in, uint32_t pow)
{
	Interest cur_base = in;
	Interest result = Interest(asset(1, in.base.asset_id), asset(1, in.quote.asset_id)); //start from 1:1 price
	auto multiply_of_interest = [&](Interest a, Interest b)
	{
		uint128_t result = uint128_t(FCC_INTEREST_BASE_SUPPLY) * uint128_t(a.quote.amount.value) / uint128_t(a.base.amount.value)  * uint128_t(b.quote.amount.value) / uint128_t(b.base.amount.value);
		return Interest(asset(FCC_INTEREST_BASE_SUPPLY, a.base.asset_id), asset(result.convert_to<uint64_t>(), b.quote.asset_id));
	};

	while (pow)
	{
		if (pow & 0x01)
			result = multiply_of_interest(result,cur_base);
		
		cur_base = multiply_of_interest(cur_base,cur_base);

		pow >>= 1;
	}
	return result;
}

Interest asset_lock_data_object::_get_interest(uint32_t lock_period,const database &_db)const{
   
   asset_object target_asset_obj=asset_id(_db);
   int32_t lock_days=lock_period/FCC_INTEREST_DAY;   
   share_type max_to_deposit_balance_year = target_asset_obj.dynamic_data(_db).current_supply - (lock_coin_day / coin_day(FCC_INTEREST_YEAR)).value.to_uint64();
   asset  base_asset(FCC_INTEREST_BASE_SUPPLY, asset_id);
   
   Interest		top_of_interest = fast_pow_of_interest(nominal_interest_perday, FCC_INTEREST_DAYS_YEAR);
   
   int32_t pecent_of_year = (lock_days - FCC_INTEREST_DAYS_YEAR)*GRAPHENE_100_PERCENT / FCC_INTEREST_DAYS_YEAR;

   int64_t reward_rate = GRAPHENE_100_PERCENT + pecent_of_year * reward_coefficient / GRAPHENE_100_PERCENT;
   
   uint128_t pre_profile = uint128_t((base_asset * fast_pow_of_interest(nominal_interest_perday,lock_days) - base_asset).amount.value)*uint128_t(reward_rate) / GRAPHENE_100_PERCENT;
   
   uint128_t max_need_pool;
   {
      uint128_t quote=uint128_t(top_of_interest.quote.amount.value);
      uint128_t base=uint128_t(top_of_interest.base.amount.value);
      uint128_t max_to_deposit=uint128_t(max_to_deposit_balance_year.value);
      
      FC_ASSERT(quote>=base," year interest  must >1");       // quote>base ,don`t worry overflow

      max_need_pool=(quote*max_to_deposit)/base-max_to_deposit;
   }

   auto actual_profile = pre_profile * uint128_t(interest_pool.value) / max_need_pool;

   asset res_asset = asset(actual_profile.convert_to<uint64_t>(), asset_id) + base_asset;

   return Interest(base_asset,res_asset);
}

share_type asset_bitasset_data_object::max_force_settlement_volume(share_type current_supply) const
{
   if( options.maximum_force_settlement_volume == 0 )
      return 0;
   if( options.maximum_force_settlement_volume == GRAPHENE_100_PERCENT )
      return current_supply + force_settled_volume;

   fc::uint128 volume = current_supply.value + force_settled_volume.value;
   volume *= options.maximum_force_settlement_volume;
   volume /= GRAPHENE_100_PERCENT;
   return volume.to_uint64();
}

void graphene::chain::asset_bitasset_data_object::update_median_feeds(time_point_sec current_time)
{
   current_feed_publication_time = current_time;
   vector<std::reference_wrapper<const price_feed>> current_feeds;
   for( const pair<account_id_type, pair<time_point_sec,price_feed>>& f : feeds )
   {
      if( (current_time - f.second.first).to_seconds() < options.feed_lifetime_sec &&
          f.second.first != time_point_sec() )
      {
         current_feeds.emplace_back(f.second.second);
         current_feed_publication_time = std::min(current_feed_publication_time, f.second.first);
      }
   }

   // If there are no valid feeds, or the number available is less than the minimum to calculate a median...
   if( current_feeds.size() < options.minimum_feeds )
   {
      //... don't calculate a median, and set a null feed
      current_feed_publication_time = current_time;
      current_feed = price_feed();
      return;
   }
   if( current_feeds.size() == 1 )
   {
      current_feed = std::move(current_feeds.front());
      return;
   }

   // *** Begin Median Calculations ***
   price_feed median_feed;
   const auto median_itr = current_feeds.begin() + current_feeds.size() / 2;
#define CALCULATE_MEDIAN_VALUE(r, data, field_name) \
   std::nth_element( current_feeds.begin(), median_itr, current_feeds.end(), \
                     [](const price_feed& a, const price_feed& b) { \
      return a.field_name < b.field_name; \
   }); \
   median_feed.field_name = median_itr->get().field_name;

   BOOST_PP_SEQ_FOR_EACH( CALCULATE_MEDIAN_VALUE, ~, GRAPHENE_PRICE_FEED_FIELDS )
#undef CALCULATE_MEDIAN_VALUE
   // *** End Median Calculations ***

   current_feed = median_feed;
}



asset asset_object::amount_from_string(string amount_string) const
{ try {
   bool negative_found = false;
   bool decimal_found = false;
   for( const char c : amount_string )
   {
      if( isdigit( c ) )
         continue;

      if( c == '-' && !negative_found )
      {
         negative_found = true;
         continue;
      }

      if( c == '.' && !decimal_found )
      {
         decimal_found = true;
         continue;
      }

      FC_THROW( (amount_string) );
   }

   share_type satoshis = 0;

   share_type scaled_precision = asset::scaled_precision( precision );

   const auto decimal_pos = amount_string.find( '.' );
   const string lhs = amount_string.substr( negative_found, decimal_pos );
   if( !lhs.empty() )
      satoshis += fc::safe<int64_t>(std::stoll(lhs)) *= scaled_precision;

   if( decimal_found )
   {
      const size_t max_rhs_size = std::to_string( scaled_precision.value ).substr( 1 ).size();

      string rhs = amount_string.substr( decimal_pos + 1 );
      FC_ASSERT( rhs.size() <= max_rhs_size );

      while( rhs.size() < max_rhs_size )
         rhs += '0';

      if( !rhs.empty() )
         satoshis += std::stoll( rhs );
   }

   FC_ASSERT( satoshis <= GRAPHENE_MAX_SHARE_SUPPLY );

   if( negative_found )
      satoshis *= -1;

   return amount(satoshis);
   } FC_CAPTURE_AND_RETHROW( (amount_string) ) }

string asset_object::amount_to_string(share_type amount) const
{
   share_type scaled_precision = 1;
   for( uint8_t i = 0; i < precision; ++i )
      scaled_precision *= 10;
   assert(scaled_precision > 0);

   string result = fc::to_string(amount.value / scaled_precision.value);
   auto decimals = amount.value % scaled_precision.value;
   if( decimals )
      result += "." + fc::to_string(scaled_precision.value + decimals).erase(0,1);
   return result;
}
