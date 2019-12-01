#ifndef CONVKEY_H
#define CONVKEY_H

template<typename T, typename U>
struct ConvKeyBase
{
  using type_key_t = std::conditional_t<std::is_same_v<T,ControllerEvent>, ControllerEvent, U>;
  
  static           type_key_t key;
  static constexpr size_t     SIZE = sizeof(U);
};

template<typename T, int N>
struct ConvKey;

template<typename T>
struct ConvKey<T, MOUSE> : public ConvKeyBase<T, struct scnp_movement>
{
  using ConvKeyBase<T,struct scnp_movement>::key;

  template<typename U>
  static T* get(const U& ev) {
    if constexpr (std::is_same_v<U, struct scnp_packet>) {
       auto pkt = reinterpret_cast<const struct scnp_movement *>(&ev);

       key.controller_type = MOUSE;
       key.ev_type = pkt->type;
       key.code = pkt->code;
       key.value = pkt->value;

       return &key;
    }
    else if(std::is_same_v<U,ControllerEvent>) {
      key.type = ev.ev_type;
      key.code = ev.code;
      key.value = ev.value;
      return reinterpret_cast<scnp_packet *>(&key);
    }
    else return nullptr;
  }
};

template<typename T>
struct ConvKey<T, KEY> : public ConvKeyBase<T, struct scnp_key>
{
  using ConvKeyBase<T,struct scnp_key>::key;
  
  template<typename U>
  static T* get(const U& ev) {

    if constexpr(std::is_same_v<U,struct scnp_packet>) {
      auto pkt = reinterpret_cast<const struct scnp_key *>(&ev);

      key.controller_type = KEY;
      key.ev_type = pkt->type;
      key.code = pkt->code;
      key.value = (pkt->flags >> 7) & 0x01;

      return &key;
    }
    else if(std::is_same_v<U,ControllerEvent>) {
      key.type = ev.ev_type;
      key.code = ev.code;
      key.flags = ev.value << 7;
      return reinterpret_cast<scnp_packet *>(&key);
    }
    else return nullptr;
  }
};

template<typename T, typename U>
typename ConvKeyBase<T,U>::type_key_t ConvKeyBase<T,U>::key;

#endif /* CONVKEY_H */
