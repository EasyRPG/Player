/** **********************************************************************
 *  ██╗     ███████╗ █████╗ ███████╗██╗   ██╗
 *  ██║     ██╔════╝██╔══██╗██╔════╝╚██╗ ██╔╝
 *  ██║     █████╗  ███████║███████╗ ╚████╔╝
 *  ██║     ██╔══╝  ██╔══██║╚════██║  ╚██╔╝
 *  ███████╗███████╗██║  ██║███████║   ██║
 *  ╚══════╝╚══════╝╚═╝  ╚═╝╚══════╝   ╚═╝
 *
 *          The EasyRPG engine, with runtime extensions, easily.
 *
 *  Developed by @wys
 *  https://github.com/wys-prog
 * 
 *  This file is free and open source. You may credit its usage in sources
 *  by using this Github profile: https://github.com/wys-prog.
 * 
 *  You may see the evolution of this file at https://github.com/wys-prog/leasy.
 * 
 *  0xEF9087A@wys-prog.https://github.com/wys-prog/leasy
 * 
 * **********************************************************************/

//
// Created by @wys on 02/08/2026.
//

#ifndef EASYRPG_PLAYER_CURSOR_HPP
#define EASYRPG_PLAYER_CURSOR_HPP
#pragma once

#include <algorithm>
#include <iterator>
#include <vector>

namespace leasy::iky7 {

  template <typename Iterator>
  class Cursor {
  public:
    using iterator = Iterator;
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using reference = typename std::iterator_traits<Iterator>::reference;

    Cursor(Iterator begin, Iterator end)
        : begin_(begin), end_(end), current_(begin) {}

    reference operator*() const {
      return *current_;
    }

    bool advance() {
      if (current_ == end_)
        return false;

      ++current_;
      return current_ != end_;
    }

    bool ended() const {
      return current_ == end_;
    }

    bool valid() const {
      return current_ != end_;
    }

    bool backward() {
      if (current_ == begin_)
        return false;

      --current_;
      return true;
    }

    template <typename Pred>
    iterator first(Pred pred) {
      current_ = std::find_if(begin_, end_, pred);
      return current_;
    }

    template <typename Pred>
    iterator last(Pred pred) {
      auto r = std::find_if(
          std::make_reverse_iterator(end_),
          std::make_reverse_iterator(begin_),
          pred);

      current_ = (r == std::make_reverse_iterator(begin_))
          ? end_
          : std::prev(r.base());

      return current_;
    }

    template <typename Func>
    void foreach(Func&& f) {
      std::for_each(begin_, end_, std::forward<Func>(f));
    }

    template <typename Pred>
    std::vector<iterator> select(Pred pred) const {
      std::vector<iterator> out;
      for (auto it = begin_; it != end_; ++it)
        if (pred(*it))
          out.push_back(it);
      return out;
    }

    iterator current() const { return current_; }

  protected:
    Iterator begin_;
    Iterator end_;
    Iterator current_;
  };

  template <typename Iterator>
  Cursor<Iterator> make_cursor(Iterator begin, Iterator end) {
    return Cursor<Iterator>(begin, end);
  }

  template <typename Container>
  auto make_cursor(Container& c) {
    return make_cursor(std::begin(c), std::end(c));
  }
} // namespace leasy::iky7

#endif //EASYRPG_PLAYER_CURSOR_HPP
