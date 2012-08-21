<?php

  /**
   * Extend the defaults.
   */
class InviteMetadataController extends EntityDefaultMetadataController {
  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];
    dpm($info);

    $properties['type'] = array(
      'setter callback' => 'entity_property_verbatim_set',
      'required' => TRUE,
      'description' => t('The invite type.'),
      'type' => 'invite_type',
    ) + $properties['type'];

    unset($properties['arguments']);
    // Unset the uid property, as it is available via the user anyway.
    unset($properties['uid']);

    $properties['inviter'] = array(
      'label' => t("Inviter"),
      'type' => 'user',
      'description' => t("The user who sent invitation."),
      'getter callback' => 'entity_property_getter_method',
      'setter callback' => 'entity_property_setter_method',
      'schema field' => 'uid',
      'required' => TRUE,
    );

    $properties['invitee_user'] = array(
      'label' => t("Invitee"),
      'type' => 'user',
      'description' => t("The user registered with invitation."),
      'getter callback' => 'entity_property_getter_method',
      'setter callback' => 'entity_property_setter_method',
      'schema field' => 'invitee',
      'required' => TRUE,
    );

    $properties['created'] = array(
      'type' => 'date',
      'setter callback' => 'entity_property_verbatim_set',
      'description' => t('The time the invite has been created.'),
    ) + $properties['created'];

    $properties['expiry'] = array(
      'type' => 'date',
      'setter callback' => 'entity_property_verbatim_set',
      'description' => t('The time invite will be expired.'),
    ) + $properties['expiry'];

    $properties['joined'] = array(
      'type' => 'date',
      'setter callback' => 'entity_property_verbatim_set',
      'description' => t('The time user have joined.'),
    ) + $properties['joined'];

    dpm($info);
    return $info;

  }
}
