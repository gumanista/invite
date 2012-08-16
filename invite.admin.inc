<?php

/**
 * Generates the invite type editing form.
 */
function invite_type_form($form, &$form_state, $task_type, $op = 'edit') {

  if ($op == 'clone') {
    $task_type->label .= ' (cloned)';
    $task_type->type = '';
  }

  $form['label'] = array(
    '#title' => t('Label'),
    '#type' => 'textfield',
    '#default_value' => $task_type->label,
    '#description' => t('The human-readable name of this invite type.'),
    '#required' => TRUE,
    '#size' => 30,
  );

  // Machine-readable type name.
  $form['type'] = array(
    '#type' => 'machine_name',
    '#default_value' => isset($task_type->type) ? $task_type->type : '',
    '#maxlength' => 32,
    '#disabled' => $task_type->isLocked() && $op != 'clone',
    '#machine_name' => array(
      'exists' => 'invite_types',
      'source' => array('label'),
    ),
    '#description' => t('A unique machine-readable name for this invite type. It must only contain lowercase letters, numbers, and underscores.'),
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#default_value' => isset($task_type->description) ? $task_type->description : '',
    '#description' => t('Description about the invite type.'),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save invite type'),
    '#weight' => 40,
  );

  if (!$task_type->isLocked() && $op != 'add' && $op != 'clone') {
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
 * Submit handler for creating/editing task_type.
 */
function invite_type_form_submit(&$form, &$form_state) {
  $task_type = entity_ui_form_submit_build_entity($form, $form_state);
  // Save and go back.
  invite_type_save($task_type);

  // Redirect user back to list of invite types.
  $form_state['redirect'] = 'admin/structure/invite-types';
}

function invite_type_form_submit_delete(&$form, &$form_state) {
  $form_state['redirect'] = 'admin/structure/invite-types/' . $form_state['invite_type']->type . '/delete';
}

/**
 * invite type delete form.
 */
function invite_type_form_delete_confirm($form, &$form_state, $task_type) {
  $form_state['task_type'] = $task_type;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['task_type_id'] = array('#type' => 'value', '#value' => entity_id('invite_type' ,$task_type));
  return confirm_form($form,
    t('Are you sure you want to delete invite type %title?', array('%title' => entity_label('task_type', $task_type))),
    'task/' . entity_id('invite_type' ,$task_type),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * invite type delete form submit handler.
 */
function invite_type_form_delete_confirm_submit($form, &$form_state) {
  $task_type = $form_state['task_type'];
  invite_type_delete($task_type);

  watchdog('invite_type', '@type: deleted %title.', array('@type' => $task_type->type, '%title' => $task_type->label));
  drupal_set_message(t('@type %title has been deleted.', array('@type' => $task_type->type, '%title' => $task_type->label)));

  $form_state['redirect'] = 'admin/structure/invite-types';
}
