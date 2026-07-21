#ifndef IVL_vvp_aarray_H
#define IVL_vvp_aarray_H
/*
 * Copyright (c) 2026 Icarus UVM track
 *
 * Associative-array runtime (string-keyed vertical slice).
 * Deferred: int keys, class elements, nested AAs.
 */

# include  "vvp_object.h"
# include  "vvp_net.h"
# include  <map>
# include  <string>
# include  <vector>

class vvp_aarray : public vvp_object {

    public:
      inline vvp_aarray() { }
      virtual ~vvp_aarray() override;

      virtual size_t get_size(void) const =0;
      virtual void clear(void) =0;

      virtual bool exists(const std::string&key) const =0;
      virtual void erase(const std::string&key) =0;

      virtual void set_word(const std::string&key, const vvp_vector4_t&value);
      virtual void get_word(const std::string&key, vvp_vector4_t&value) const;

      virtual std::string key_at(size_t idx) const =0;
};

class vvp_aarray_vec4 : public vvp_aarray {

    public:
      explicit vvp_aarray_vec4(unsigned word_wid);
      ~vvp_aarray_vec4() override;

      size_t get_size(void) const override;
      void clear(void) override;
      bool exists(const std::string&key) const override;
      void erase(const std::string&key) override;

      void set_word(const std::string&key, const vvp_vector4_t&value) override;
      void get_word(const std::string&key, vvp_vector4_t&value) const override;

      std::string key_at(size_t idx) const override;

      void shallow_copy(const vvp_object*obj) override;
      vvp_object* duplicate(void) const override;

      unsigned word_wid() const { return word_wid_; }

    private:
      std::map<std::string, vvp_vector4_t> map_;
      unsigned word_wid_;
};

#endif /* IVL_vvp_aarray_H */
