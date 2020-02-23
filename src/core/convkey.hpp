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

  /*
    *\brief Convert scnp_packet into ControllerEvent
  */
  static Dest * get(const struct scnp_packet& ev)
  {
    auto pkt = reinterpret_cast<const Pkt *>(&ev);

    key.controller_type = Impl::CTRL_TYPE;
    key.code = pkt->code;
    
    Impl::set(pkt);

    return &key;
  }

  /*
    *\brief Convert ControllerEvent into scnp_packet
  */

  static Dest * get(const ControllerEvent& ev)
  {
    key.type = Impl::SCNP_TYPE;
    key.code = ev.code;

    Impl::set(ev);
      
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
  static void set(const U* packet) {
    key.ev_type = (packet->move_type == MOV_ABS) ? EV_ABS : EV_REL;
    key.value = packet->value;
  }

  template<typename U>
  static void set(const U& ev) {
    key.move_type = (ev.ev_type == EV_ABS) ? MOV_ABS : MOV_REL;
    key.value = ev.value;
  }
};

template<typename T>
struct ConvKey<T, KEY> : public ConvKeyBase<ConvKey<T,KEY>,T,struct scnp_key>
{
  using ConvKeyBase<ConvKey<T,KEY>,T,struct scnp_key>::key;
  
  static constexpr int CTRL_TYPE=KEY;
  static constexpr int CTRL_EV_TYPE=EV_KEY;
  static constexpr int SCNP_TYPE=SCNP_KEY;
  
  template<typename U>
  static void set(const U& ev) {
    if(ev.value == KEY_REPEATED) {
      key.pressed = true;
      key.repeated = true;
    }
    else {
      key.pressed = ev.value == KEY_PRESSED;
      key.repeated = false;
    }
  }

  template<typename U>
  static void set(const U * packet) {
    key.ev_type = CTRL_EV_TYPE;
    if(packet->repeated) key.value = KEY_REPEATED;
    else                 key.value = packet->pressed;
  }
};

template<typename T,typename U, typename V>
typename ConvKeyBase<T,U,V>::type_key_t ConvKeyBase<T,U,V>::key;

#endif /* CONVKEY_H */
