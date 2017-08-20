# digest-xxhash-ruby

This gem 'digest-xxhash' provides XXH32 and XXH64 functions for Ruby.  It works
on top of Digest::Class and complies with the functional design of
Digest::Instance.

Its core implementation was taken from the official source, which is
in https://github.com/Cyan4973/xxHash.

## Installation

Add this line to your application's Gemfile:

    gem 'digest-xxhash'

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install digest-xxhash

## Example Usage

    require 'digest/xxhash'

    Digest::XXH32.digest("ABXY")
    => "\x0E\xA4\xB6\xCA"

    Digest::XXH32.hexdigest("ABXY")
    => "0ea4b6ca"

    Digest::XXH32.idigest("ABXY")
    => 245675722

    Digest::XXH32.hexdigest("1234", "\xab\xcd\xef\x00")
    => "2af3be26"

    Digest::XXH32.hexdigest("1234", "abcdef00")
    => "2af3be26"

    Digest::XXH32.hexdigest("1234", 2882400000)
    => "2af3be26"

    Digest::XXH64.hexdigest("1234", "0123456789abcdef")
    => "d7544504de216507"

    Digest::XXH64.new("0123456789abcdef").update("1234").hexdigest
    => "d7544504de216507"

    Digest::XXH64.new.reset("0123456789abcdef").update("12").update("34").hexdigest
    => "d7544504de216507"

## Contributing

1. Fork it ( https://github.com/konsolebox/digest-xxhash-ruby/fork ).
2. Create your feature branch (`git checkout -b my-new-feature`).
3. Commit your changes (`git commit -am 'Add some feature'`).
4. Push to the branch (`git push origin my-new-feature`).
5. Create a new Pull Request.
