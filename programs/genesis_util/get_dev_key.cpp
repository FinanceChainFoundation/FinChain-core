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

#include <iostream>
#include <string>

#include <fc/crypto/elliptic.hpp>
#include <fc/io/json.hpp>

#include <graphene/chain/protocol/address.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/utilities/key_conversion.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#ifndef WIN32
#include <csignal>
#endif

using namespace std;

int main( int argc, char** argv )
{
   try
   {
      std::string dev_key_prefix;
      bool need_help = false;
      if( argc < 2 )
         need_help = true;
      else
      {
         dev_key_prefix = argv[1];
         if(  (dev_key_prefix == "-h")
           || (dev_key_prefix == "--help")
           )
           need_help = true;
      }

      if( need_help )
      {
         std::cerr << argc << " " << argv[1]  << "\n";
         std::cerr << "get-dev-key <prefix> <suffix> ...\n"
             "\n"
             "example:\n"
             "\n"
             "get-dev-key wxyz- owner-5 active-7 balance-9 wit-block-signing-3 wit-owner-5 wit-active-33\n"
             "get-dev-key wxyz- wit-block-signing-0:101\n"
             "\n";
         return 1;
      }

      bool comma = false;

      auto show_key = [&]( const fc::ecc::private_key& priv_key )
      {
         fc::mutable_variant_object mvo;
         graphene::chain::public_key_type pub_key = priv_key.get_public_key();
         mvo( "private_key", graphene::utilities::key_to_wif( priv_key ) )
            ( "public_key", std::string( pub_key ) )
            ( "address", graphene::chain::address( pub_key ) )
            ;
         if( comma )
            std::cout << ",\n";
         std::cout << fc::json::to_string( mvo );
         comma = true;
      };

      std::cout << "[";

      string salt;
      uint32_t n;
      cout<<"slat:";
      cin>>salt;
      cout<<"n:";
      cin>>n;
      
      vector< fc::ecc::private_key > keys;
      
      fc::sha256 saltHash=fc::sha256::hash(salt);
      for( int i=0; i<n; i++ )
      {
         fc::sha256 randomHash;
         boost::random_device random_device;
         for(uint i=0;i<4;i++)
         {
            for(uint j=0;j<8;j++)
            if(j)
               randomHash._hash[i]=randomHash._hash[i]<<8;
            randomHash._hash[i]+= (uint8_t)boost::random::uniform_int_distribution<uint16_t>(0, 255)(random_device);

         }
         
         fc::sha256 serect =fc::sha256::hash(randomHash.str()+salt);

         keys.push_back( fc::ecc::private_key::regenerate( serect));
         
      }
      for(const auto &key:keys)
         show_key(key);
      std::cout << "\n";
   }
   catch ( const fc::exception& e )
   {
      std::cout << e.to_detail_string() << "\n";
      return 1;
   }
   return 0;
}
