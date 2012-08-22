<?php

/**
 * Generates the invite type editing form.
 */
function invite_type_form($form, &$form_state, $invite_type, $op = 'edit') {
  if ($op == 'clone') {
    $invite_type->label .= ' (cloned)';
    $invite_type->type = '';
  }
//  // Load all plugins type "operation".
//  ctools_include('plugins');
//  $operations = ctools_get_plugins('invite', 'invite_type');
//
//  $operation_options = array();
//  foreach ($operations as $id => $operation) {
//    $operation_options[$id] = $operation['label'];
//  }
//
  $invite_processors = array();
  foreach (module_implements('invite_processor') as $module) {
    $processor = module_invoke($module, 'invite_processor');
    $invite_processors[$module] = $processor['label'];
  }

  if (empty($invite_processors)) {
    $form['message'] = array(
      '#markup' => t('Sorry no Invitation Type plugins available in the system.'),
    );
    return $form;
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $invite_type->label,
    '#description' => t('The human-readable name of this invite type.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($invite_type->type) ? $invite_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $invite_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'invite_get_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this invite type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['invite_processor'] = array(
    '#title' => t('Invite type'),
    '#type' => 'select',
    '#default_value' => !empty($invite_type->invite_processor) ? $invite_type->invite_processor : NULL,
    '#disabled' => !empty($invite_type->invite_processor),
    '#description' => t('Invitation processing plugin.'),
    '#options' => $invite_processors,
    '#required' => TRUE,
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#default_value' => isset($invite_type->description) ? $invite_type->description : '',
    '#description' => t('Description about the invite type.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save invite type'),
    '#weight' => 40,
  );

  if (!$invite_type->isLocked() && $op != 'add' && $op != 'clone') {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete invite type'),
      '#weight' => 45,
      '#limit_validation_errors' => array(),
      '#submit' => array('invite_type_form_submit_delete')
    );
  }
  return $form;
}

/**
 * Submit handler for creating/editing invite_type.
 */
function invite_type_form_submit(&$form, &$form_state) {
  $invite_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  invite_type_save($invite_type);

  // Redirect user back to list of invite types.
  $form_state['redirect'] = 'admin/structure/invite-types';
}

function invite_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/invite-types/' . $form_state['invite_type']->type . '/delete';
}

/**
 * Invite type delete form.
 */
function invite_type_form_delete_confirm($form, &$form_state, $invite_type) {
  $form_state['invite_type'] = $invite_type;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['invite_type_id'] = array('#type' => 'value', '#value' => entity_id('invite_type' ,$invite_type));
  return confirm_form($form,
    t('Are you sure you want to delete invite type %title?', array('%title' => entity_label('invite_type', $invite_type))),
    'task/' . entity_id('invite_type' ,$invite_type),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Invite type delete form submit handler.
 */
function invite_type_form_delete_confirm_submit($form, &$form_state) {
  $invite_type = $form_state['invite_type'];
  invite_type_delete($invite_type);

  watchdog('invite_type', '@type: deleted %title.', array('@type' => $invite_type->type, '%title' => $invite_type->label));
  drupal_set_message(t('@type %title has been deleted.', array('@type' => $invite_type->type, '%title' => $invite_type->label)));

  $form_state['redirect'] = 'admin/structure/invite-types';
}


/**
 * Page to select invite Type to add new invite.
 */
function invite_admin_add_page() {
  $items = array();
  foreach (invite_get_types() as $invite_type_key => $invite_type) {
    $items[] = l(entity_label('invite_type', $invite_type), 'invite/add/' . $invite_type_key);
  }
  return array('list' => array('#theme' => 'item_list', '#items' => $items, '#title' => t('Select type of invite to create.')));
}

/**
 * Add new invite page callback.
 */
function invite_add($type) {
  $invite_type = invite_get_types($type);

  $invite = entity_create('invite', array('type' => $type));
  drupal_set_title(t('Create @name', array('@name' => entity_label('invite_type', $invite_type))));

  $output = drupal_get_form('invite_form', $invite);

  return $output;
}

/**
 * Invite Form.
 */
function invite_form($form, &$form_state, $invite) {
  $form_state['invite'] = $invite;

//  $form['description'] = array(
//    '#type' => 'textarea',
//    '#title' => t('Description'),
//    '#default_value' => $invite->description,
//  );

  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $invite->uid,
  );

  field_attach_form('invite', $invite, $form, $form_state);

  $submit = array();
  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions'] = array(
    '#weight' => 100,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save invite'),
    '#submit' => $submit + array('invite_form_submit'),
  );

  // Show Delete button if we edit invite.
  $invite_id = entity_id('invite' ,$invite);
  if (!empty($invite_id) && invite_access('edit', $invite)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('invite_form_submit_delete'),
    );
  }

  $form['#validate'][] = 'invite_form_validate';

  return $form;
}

/**
 * Validates invitation form.
 *
 * @param $form
 * @param $form_state
 */
function invite_form_validate($form, &$form_state) {

}

/**
 * invite submit handler.
 */
function invite_form_submit($form, &$form_state) {
  $invite = $form_state['invite'];

  entity_form_submit_build_entity('invite', $invite, $form, $form_state);

  if (invite_save($invite)) {
    $invite_uri = entity_uri('invite', $invite);
    $form_state['redirect'] = $invite_uri['path'];
    drupal_set_message(t('Invite %title added.', array('%title' => entity_label('invite', $invite))));
  }
  else {
    drupal_set_message(t('Saving invite %title failed.', array('%title' => entity_label('invite', $invite))), 'error');
  }
}

function invite_form_submit_delete($form, &$form_state) {
  $invite = $form_state['invite'];
  $invite_uri = entity_uri('invite', $invite);
  $form_state['redirect'] = $invite_uri['path'] . '/delete';
}

/**
 * Delete confirmation form.
 */
function invite_delete_form($form, &$form_state, $invite) {
  $form_state['invite'] = $invite;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['invite_type_id'] = array('#type' => 'value', '#value' => entity_id('invite' ,$invite));
  $invite_uri = entity_uri('invite', $invite);
  return confirm_form($form,
    t('Are you sure you want to delete invite %title?', array('%title' => entity_label('invite', $invite))),
    $invite_uri['path'],
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Delete form submit handler.
 */
function invite_delete_form_submit($form, &$form_state) {
  $invite = $form_state['invite'];
  invite_delete($invite);

  drupal_set_message(t('invite %title deleted.', array('%title' => entity_label('invite', $invite))));

  $form_state['redirect'] = '<front>';
}


function invite_settings_form() {
  $form = array();

  $form['invite_default_expiry_time'] = array(
    '#type' => 'select',
    '#title' => t('Invitation expiry'),
    '#default_value' => variable_get('invite_default_expiry_time', 30),
    '#options' => drupal_map_assoc(array(1, 3, 7, 14, 30, 60, 365)),
    '#description' => t('Set the expiry period for user invitations, in days.'),
    '#multiple' => FALSE,
    '#required' => TRUE,
  );

  $form['invite_registration_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to registration page'),
    '#default_value' => variable_get('invite_registration_path', 'user/register'),
    '#description' => t('Path to the registration page for invited users. Useful when using the <em>Assign from Path</em> option of <a href="@url">Auto Assign Roles</a> module.', array('@url' => 'http://drupal.org/project/autoassignrole')),
  );

  $form['invite_require_approval'] = array(
    '#type' => 'checkbox',
    '#title' => t('Require administrator approval for invitees'),
    '#default_value' => variable_get('invite_require_approval', FALSE),
    '#description' => t('Accounts that have been created with an invitation will require administrator approval.'),
  );

  return system_settings_form($form);
}