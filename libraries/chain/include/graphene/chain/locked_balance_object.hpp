/*
 * Copyright (c) 2017 FinChain, Inc., and contributors.
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
#include <graphene/chain/protocol/operations.hpp>
#include <graphene/db/generic_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <math.h>

namespace graphene { namespace chain {
   using namespace graphene::db;
   class database;
   
   /**
    * @class locked_balance_object
    * @ingroup object
    * @ingroup implementation
    *
    */
   class locked_balance_object : public abstract_object<locked_balance_object>
   {
   public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id  = locked_balance_object_type;
      
      enum LockType{
         genesis,
         userSet
      };
      
      share_type  initial_lock_balance;
      share_type  locked_balance;
      TimeStamp   lock_time;
      TimeStamp   lock_period;
      LockType    lock_type;
	  asset_id_type asset_id;
	  bool		  finish = false;
      
      uint64_t get_interest()const{
         share_type profile=(locked_balance-initial_lock_balance)/initial_lock_balance;
         return uint64_t(pow(double(profile.value),double(lock_period.value/(3600*24))));
      }
      
      uint32_t get_unlock_time()const {
         return (lock_time+lock_period).value;
      }
   };
   //struct by_id{};
   
   /**
    * @ingroup object_index
    */
   /*
   typedef multi_index_container<
      locked_balance_object,
      indexed_by<
         ordered_unique< tag<by_id>, member< object, object_id_type, &object::id > >
      >
   > locked_balance_index_type;
   

   typedef generic_index<locked_balance_object, locked_balance_index_type> locked_balance_index;
    */
}}

FC_REFLECT_ENUM(graphene::chain::locked_balance_object::LockType,(genesis)(userSet))

FC_REFLECT_DERIVED( graphene::chain::locked_balance_object,
                   (graphene::db::object),
                   (initial_lock_balance)(locked_balance)(lock_time)(lock_period)(lock_type)
                   )
