#ifndef CONVKEY_H
#define CONVKEY_H

template<typename Impl, typename Dest, typename Pkt>
struct ConvKeyBase
{
  using type_key_t = std::conditional_t<std::is_same<Dest,ControllerEvent>::value,
					ControllerEvent,
					Pkt>;
  
  static           type_key_t key;
  static constexpr size_t     SIZE = sizeof(Pkt);

  static Dest * get(const struct scnp_packet& ev)
  {
    auto pkt = reinterpret_cast<const Pkt *>(&ev);

    key.controller_type = Impl::CTRL_TYPE;
    key.ev_type = pkt->type;
    key.code = pkt->code;
    
    Impl::set_value(pkt);

    return &key;
  }

  static Dest * get(const ControllerEvent& ev)
  {
    key.type = Impl::SCNP_TYPE;
    key.code = ev.code;

    Impl::set_value(ev);
      
    return reinterpret_cast<scnp_packet *>(&key);
  }

};

template<typename T, int N>
struct ConvKey;

template<typename T>
struct ConvKey<T, MOUSE> : public ConvKeyBase<ConvKey<T,MOUSE>,T,struct scnp_movement>
{
  using ConvKeyBase<ConvKey<T,MOUSE>,T,struct scnp_movement>::key;
  
  static constexpr int CTRL_TYPE=MOUSE;
  static constexpr int SCNP_TYPE=SCNP_MOV;
  
  template<typename U>
  static void set_value(const U* u) {
    key.value = u->value;
  }

  template<typename U>
  static void set_value(const U& u) {
    key.value = u.value;
  }
};

template<typename T>
struct ConvKey<T, KEY> : public ConvKeyBase<ConvKey<T,KEY>,T,struct scnp_key>
{
  using ConvKeyBase<ConvKey<T,KEY>,T,struct scnp_key>::key;
  
  static constexpr int CTRL_TYPE=KEY;
  static constexpr int SCNP_TYPE=SCNP_KEY;
  
  template<typename U>
  static void set_value(const U& val) {
    key.pressed = val.value == KEY_PRESSED;
  }

  template<typename U>
  static void set_value(const U * val) {
    key.value = val->pressed;
  }
};

template<typename T,typename U, typename V>
typename ConvKeyBase<T,U,V>::type_key_t ConvKeyBase<T,U,V>::key;

#endif /* CONVKEY_H */
