/*
	Copyright 2005-2007 Adobe Systems Incorporated
	Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
	or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <adobe/config.hpp>

#include <functional>
#include <utility>

#define BOOST_TEST_MAIN

#include <boost/test/unit_test.hpp>

#include <adobe/conversion.hpp>

/*************************************************************************************************/

namespace {

struct base { virtual ~base() { } };
struct derived : base { };
struct not_derived { };
struct other_derived : base { };

} // namespace


BOOST_AUTO_TEST_CASE(conversion_test) {
    using adobe::runtime_cast;

    {
    base* x = new derived;
    BOOST_CHECK(runtime_cast<derived*>(x));
    BOOST_CHECK(runtime_cast<const derived*>(x));
    // BOOST_CHECK(runtime_cast<not_derived*>(x));

    BOOST_CHECK(runtime_cast<other_derived*>(x) == 0);

    runtime_cast<derived&>(*x) = derived();
    }

    {
    const base* x = new derived;
   // BOOST_CHECK(runtime_cast<derived*>(x));
    BOOST_CHECK(runtime_cast<const derived*>(x));
    derived y = runtime_cast<const derived&>(*x);
    }
}
