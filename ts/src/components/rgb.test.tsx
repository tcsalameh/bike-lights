import * as React from 'react';
import * as enzyme from 'enzyme';
import RGBSelector from './rgb';

const wrapper = enzyme.shallow(<RGBSelector title="R"/>);
let container: any, containerProp: any;

describe("rgb selector - basic", () => {
  beforeEach(() => {
    container = wrapper.find("input");
    containerProp = container.props();
  });

  it('should have an input field', () => {
    expect(container).toHaveLength(1);
  });

  it('input field has classname rgb-input', () => {
    expect(containerProp.className).toEqual("rgb-input");
  });

  it('input field contains no text to start', () => {
    expect(container.text()).toEqual("");
  });
});

  
